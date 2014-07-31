#include <errno.h>
#include <sys/mm/mm.h>

#ifdef DEBUG
void kmalloc_debug();
#endif

/* size of structure should be a quad word or multiple thereof */
struct __attribute__((packed)) chunk
{
  unsigned int __unused;
  unsigned int prev;     /* quad word offset to previous chunk */
  unsigned int next;     /* quad word offset to next chunk */
  unsigned short flags;
  unsigned short size;   /* chunk size in bytes */
};
typedef struct chunk chunk_t;

/* flag for chunk_t */
#define CHUNK_IN_USE (1<<0) /* `in use' means chunk is allocated */

#define to_external(p)                                                  \
  (void*)(((unsigned long*)p)+sizeof(chunk_t)/sizeof(unsigned long))
#define to_internal(p)                                                  \
  (chunk_t*)(((unsigned long*)p)-sizeof(chunk_t)/sizeof(unsigned long))
#define diff(a, b) (unsigned int)((unsigned long)a-(unsigned long)b)
#define min_chunk_size_qw()                                             \
  (((sizeof(unsigned long)+sizeof(chunk_t)))/sizeof(unsigned long))
#define chunk_size_qw(size)                                             \
  (size/sizeof(unsigned long)+sizeof(chunk_t)/sizeof(unsigned long))
#define is_free(chunk) ((chunk->flags&CHUNK_IN_USE)==0)
#define is_4k_aligned(chunk)                            \
  (((unsigned long)to_external(chunk))==                \
   ((((unsigned long)to_external(chunk))>>12)<<12))
#define prev_chunk(base, offset)                        \
  (chunk_t*)((unsigned long)base-(unsigned long)offset)
#define next_chunk(base, offset)                        \
  (chunk_t*)((unsigned long)base+(unsigned long)offset)

static unsigned long *pmem;
static chunk_t *head;
static chunk_t *tail;
static chunk_t *alloc_chunk(chunk_t *prev, unsigned int size);
static chunk_t *alloc_chunks(chunk_t *prev, unsigned int size, int flags);

int
kmalloc_init(mem_map_t *map)
{
  /* only considers first memory map entry */
  pmem=(unsigned long*)map->base[0];
  /* TODO use size to detect out-of-memory */
  head=0;
  tail=0;

  return 0;
}

void*
kmalloc(unsigned int size, int flags)
{
  chunk_t *ck=0;

  if (!(flags&KMALLOC_8_ALIGNED)
      &&!(flags&KMALLOC_4K_ALIGNED))
    return (-EINVAL);

  /* round up size to multiples of a quad word */
  size=(size+7)&-8;

  if (!head)
    {
      head=alloc_chunks(0, size, flags);
      ck=head;
      
      while (ck->next)
        ck=next_chunk(ck, ck->next);

      tail=ck;
    }
  else
    {
      /* Walk linked list until either
       * a) a suitably sized chunk is found, or
       * b) tail is reached.
       */
      ck=head;
      while (ck->next)
        {
          if (ck->size>=size && is_free(ck))
            {
              if (KMALLOC_8_ALIGNED==flags
                  || (KMALLOC_4K_ALIGNED==flags && is_4k_aligned(ck)))
                {
                  /* allocate by setting flag */
                  ck->flags|=CHUNK_IN_USE;
                  break;
                  /* TODO: could split chunk as future optimisation */
                }
            }
          ck=next_chunk(ck, ck->next);
        }

      if (ck==tail)
        {
          ck=alloc_chunks(ck, size, flags);

          while (ck->next)
            ck=next_chunk(ck, ck->next);

          tail=ck;
        }
    }

  return to_external(ck);
}

int
kfree(void *p)
{
  chunk_t *ck=(chunk_t*)to_internal(p);

  if (is_free(ck))
    return (-EINVAL);

  ck->flags&=(~CHUNK_IN_USE);

  chunk_t *prev=prev_chunk(ck, ck->prev);

  /* head and tail point back to themselves */
  if (ck==prev)
    return 0;

  chunk_t *next=next_chunk(ck, ck->next);

  /* combine with previous chunk when not in use */
  if (is_free(prev))
    {
      prev->size+=ck->size;
      prev->next+=ck->next;
      next->prev+=ck->prev;
    }

#ifdef DEBUG
  kmalloc_debug();
#endif

  return 0;
}

chunk_t*
alloc_chunks(chunk_t *prev, unsigned int size, int flags)
{
  if (flags&KMALLOC_4K_ALIGNED)
    {
      /* The idea is to not waste memory. If there's free memory
       * between the current top-of-heap and the next 4K boundary
       * then create a chunk but mark as free. The chunk is sized
       * such that a subsequent allocation falls on a 4K
       * boundary.
       */
      unsigned long next4k=(unsigned long)pmem;
      next4k=(next4k+4095UL)&-4096UL;
      unsigned long diff=next4k-(unsigned long)pmem;

      if (diff<min_chunk_size_qw())
        {
          next4k=(((unsigned long)pmem)+8191UL)&-8192UL;
          diff=next4k-(unsigned long)pmem;
        }

      prev=alloc_chunk(prev, diff-2*sizeof(chunk_t));
      kfree(to_external(prev));
    }

  return alloc_chunk(prev, size);
}

chunk_t*
alloc_chunk(chunk_t *prev, unsigned int size)
{
  chunk_t *ck=(chunk_t*)pmem;

  if (!prev)
    ck->prev=0;
  else
    /* calculate quad word offsets for doubly linked list */
    ck->prev=prev->next=diff(ck, prev);

  ck->next=0;
  ck->size=size;
  ck->flags=CHUNK_IN_USE;

  /* advance pointer (and thus mark as allocated) */
  pmem+=chunk_size_qw(size);

  return ck;
}

#ifdef DEBUG
void
kmalloc_debug()
{
  chunk_t *ck=head;
  int i=0;

  serial_printk("*** begin ***\r\n");

  while (ck->next)
    {
      serial_printk("%d: 0x%lx (0x%x), next=0x%lx, prev=0x%lx, sz=%d, ext=0x%lx\r\n",
                    i++,
                    (unsigned long)ck,
                    ck->flags,
                    next_chunk(ck, ck->next),
                    prev_chunk(ck, ck->prev),
                    ck->size,
                    (unsigned long)to_external(ck));
      ck=next_chunk(ck, ck->next);
    }

  serial_printk("%d: 0x%lx (0x%x), next=0x%lx, prev=0x%lx, sz=%d, ext=0x%lx\r\n",
                i,
                (unsigned long)ck,
                ck->flags,
                next_chunk(ck, ck->next),
                prev_chunk(ck, ck->prev),
                ck->size,
                (unsigned long)to_external(ck));

  serial_printk("*** end ***\r\n");
}
#endif /* #ifdef DEBUG */
