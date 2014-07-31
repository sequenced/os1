#include <stdarg.h> /* for va_list etc. */
#include <types.h>
#include <mem.h>
#include <boot.h>
#include <multiboot2.h>
#include <printk.h>
#include <sys/dev.h>
#include <sys/console.h>
#include <sys/errno.h>
#include <sys/mm/mm.h>

#define IA32_APIC_BASE_MSR     0x1b
#define LOCAL_APIC_ID_OFF      0x20
#define LOCAL_APIC_VERSION_OFF 0x30
#define LVT_LINT0_OFF          0x350
#define LVT_LINT1_OFF          0x360

#define BUFSIZE 8192
#define printk early_printk

#define ON_BUG(op,text)                                 \
  rv=(op);                                              \
  if (0>rv)                                             \
    {                                                   \
      serial_printk(text ": %s\r\n", errno(rv));        \
      halt();                                           \
    }                                                   \

/* symbol defined in linker script */
extern unsigned long _kern_end;

static chrdev console;
static char buf[BUFSIZE];
static multiboot_memory_map_t *mmap;
static int mmap_size;

static void extract_multiboot2_details(u32 addr, mem_map_t *mm, u64 *fb);
static void print_hw_details();
void early_printk(const char *fmt, ...);

void
boot_kmain(u32 magic, u32 addr)
{
  int rv;
  serial_init(CONSOLE, DEFAULT_BAUD);

  if (MULTIBOOT2_BOOTLOADER_MAGIC!=magic)
    {
      serial_printk("unexpected multiboot2 magic: 0x%x\r\nhalting\r\n", magic);
      halt();
    }

  serial_printk("x86_64\r\nboot_kmain: multiboot2 magic=0x%x\r\n", magic);

  unsigned long fb;
  mem_map_t mm;
  zeromem((void*)&mm, sizeof(mem_map_t));
  extract_multiboot2_details(addr, &mm, &fb);

  ON_BUG(mkdev(DEV_MAJOR_CONSOLE, 0, &console), "mkdev");
  console.ioctl(DEV_CONSOLE_ADDRESS, fb);
  console.ioctl(DEV_CONSOLE_CLEAR);

  printk("x86_64\r\nboot_kmain: multiboot2 magic=0x%x\r\n", magic);
  print_hw_details();

  /* Here we tweak the memory map. The linker script generates
   * the symbol _kern_end. This symbol marks the end of kernel
   * text and data. The symbol is 4KByte-aligned. We use it to
   * tell kmalloc_init where the heap starts.
   */
  mm.base[0]=(unsigned long)&_kern_end;
  mm.size[0]-=mm.base[0];
  printk("mm.base[0]=0x%lx, mm.size[0]=%d\r\n", mm.base[0], mm.size[0]);
  ON_BUG(kmalloc_init(&mm), "kmalloc_init");

  char *p1=kmalloc(256, KMALLOC_8_ALIGNED);
  char *p2=kmalloc(177, KMALLOC_4K_ALIGNED);
  char *p3=kmalloc(3, KMALLOC_8_ALIGNED);
  kmalloc(8192, KMALLOC_4K_ALIGNED);

  kfree(p1);
  kfree(p2);
  kfree(p3);
}

void
print_hw_details()
{
  printk("memory:\r\n");
  
  int i;
  for (i=0; i<mmap_size; i++)
    printk("base=0x%lx, len=0x%lx, type=0x%x\r\n",
           (mmap+i)->addr,
           (mmap+i)->len,
           (mmap+i)->type);

  u32 eax;
  u32 ebx;
  u32 ecx;
  u32 edx;
  cpuid(0x1, &eax, &ebx, &ecx, &edx);
  printk("cpuid:\r\nstepping=%d, model=%d, family=%d\r\n",
         (eax&0xf),
         (eax&0xf0),
         (eax&0xf00));
  printk("apic=%d, sep=%d, mtrr=%d\r\n",
         (edx&0x200?1:0),
         (edx&0x800?1:0),
         (edx&0x1000?1:0));

  u64 val=rdmsr(IA32_APIC_BASE_MSR);
  u32 apic_base_addr=(val&0xffffff000);
  printk("apic:\r\nbase=0x%lx, bsp=%d, enabled=%d, id=%d\r\n",
         (apic_base_addr),
         (val&0x100?1:0),
         (val&0x800?1:0),
         (ebx&0xff000000));

  /* /\* u32 k=inl(apic_base_addr+LOCAL_APIC_VERSION_OFF); *\/ */
  /* u32 k=*((volatile u32*)(apic_base_addr+LOCAL_APIC_VERSION_OFF)); */
  /* printk("version=%d, max lvt entry=%d, eoi broadcast suppressed=%d\r\n", */
  /*        (k&0xff), */
  /*        (k&0xff0000)>>16, */
  /*        (k&0x1000000)>>24); */

  /* k=*((volatile u32*)(apic_base_addr+LVT_LINT0_OFF)); */
  /* /\* k=inl(apic_base_addr+LVT_LINT0_OFF); *\/ */
  /* printk("lint0=0x%x\r\n"); */
  /* /\* k=inl(apic_base_addr+LVT_LINT1_OFF); *\/ */
  /* k=*((volatile u32*)(apic_base_addr+LVT_LINT1_OFF)); */
  /* printk("lint1=0x%x\r\n"); */
}

void
early_printk(const char *fmt, ...)
{
  va_list a;
  va_start(a, fmt);
  char *s=buf;
  int rv=vsnprintk(buf, BUFSIZE, fmt, a);

  if (rv)
    while (*s)
      {
        rv=console.write(s++, 1);
        if (0>rv)
          {
            serial_printk("early_printk: %s\r\n", errno(rv));
            break;
          }
      }

  va_end(a);
}

void
extract_multiboot2_details(u32 addr, mem_map_t *mm, u64 *fb_addr)
{
  struct multiboot_tag *tag;

  for (tag=(struct multiboot_tag*)(u64)(addr+8);
       tag->type!=MULTIBOOT_TAG_TYPE_END;
       tag=(struct multiboot_tag*)((u8*)tag+((tag->size+7)&~7)))
    {
      serial_printk("tag=0x%x, size=0x%x\r\n", tag->type, tag->size);

      switch (tag->type)
        {
        case MULTIBOOT_TAG_TYPE_MMAP:
          {
            serial_printk("memory:\r\n");

            struct multiboot_tag_mmap *m=(struct multiboot_tag_mmap*)(u32*)tag;
            multiboot_memory_map_t *e=m->entries;
            int n=m->size/m->entry_size;
            /* save away for later */
            mmap=e;
            mmap_size=n;
            int j;
            int k=0;

            for (j=0; j<n; j++, e++)
              {
                serial_printk("base=0x%lx, len=0x%lx, type=0x%x\r\n",
                              e->addr,
                              e->len,
                              e->type);

                if (MULTIBOOT_MEMORY_AVAILABLE==e->type)
                  if (k<mem_map_num())
                    {
                      mm->base[k]=e->addr;
                      mm->size[k]=e->len;
                      k++;
                    }
              }
          }
          break;
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
          {
            struct multiboot_tag_framebuffer *fb=
              (struct multiboot_tag_framebuffer*)(u32*)tag;

            /* save away for later */
            *fb_addr=fb->common.framebuffer_addr;
            
            serial_printk("framebuffer: addr=0x%lx, w=%d, h=%d type=%d\r\n",
                          fb->common.framebuffer_addr,
                          fb->common.framebuffer_width,
                          fb->common.framebuffer_height,
                          fb->common.framebuffer_type);
          }
          break;
        }
    }
}
