#ifndef __BOOT_H__
#define __BOOT_H__

#include <types.h>

#define CONSOLE 0x3f8
#define DEFAULT_BAUD 115200

void serial_init(u16 port, u32 baud);
int serial_putchar(u8 c);
int serial_printk(const char *fmt, ...);

static inline void
outb(u8 c, u16 port)
{
  /* see IA-32 instruction set reference for details on constraints */
  asm volatile("outb %0, %1" :: "a" (c), "dN" (port));
}

static inline u8
inb(u16 port)
{
  u8 c;
  asm volatile("inb %1, %0" : "=a" (c) : "dN" (port));
  return c;
}

static inline u32
inl(u16 port)
{
  u32 c;
  asm volatile("inl %1, %0" : "=a" (c) : "dN" (port));
  return c;
}

static inline void
cpuid(u32 op, u32 *a, u32 *b, u32 *c, u32 *d)
{
  asm volatile("cpuid"
               : "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d)
               : "0" (op)
               : "memory");
}

static inline u64
rdmsr(u32 reg)
{
  u64 rv;
  u32 hi;
  u32 lo;
  asm volatile("rdmsr"
               : "=a" (lo), "=d" (hi)
               : "c" (reg)
               : "memory");
  rv=hi;
  rv=(rv<<32);
  rv|=(lo&0xffffffff);
  return rv;
}

static inline
void halt()
{
  asm volatile("hlt"::);
}

#endif /* #ifndef __BOOT_H__ */
