#ifndef __MM_H__
#define __MM_H__

/* flags for kmalloc() */
#define KMALLOC_8_ALIGNED  (1<<0)
#define KMALLOC_4K_ALIGNED (1<<1)

#define MEM_MAP_NUM_MAX 4
#define mem_map_num() (MEM_MAP_NUM_MAX)
struct mem_map
{
  unsigned long base[MEM_MAP_NUM_MAX];
  unsigned long size[MEM_MAP_NUM_MAX];
};
typedef struct mem_map mem_map_t;

int kmalloc_init(mem_map_t *map);
void* kmalloc(unsigned int size, int flags);
int kfree(void *p);

#endif /* #ifndef __MM_H__ */
