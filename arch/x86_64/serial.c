#include <printk.h>
#include <arch/x86_64/boot.h>
#include <arch/x86_64/types.h>

/* defines for Intel 8251A (or newer) */
#define DEFAULT_BAUD_RATE 9600
#define DLAB 0x80
#define TXRDY 0x20
#define THR 0 /* Transmitter Holding Buffer (write) */
#define RBR 0 /* Receiver Buffer (read) */
#define DLL 0 /* Divisor Latch Low Byte */
#define IER 1 /* Interrupt Enable Register */
#define DLH 1 /* Divisor Latch High Byte */
#define IIR 2 /* Interrupt Indentification Register */
#define FCR 2 /* FIFO Control Register */
#define LCR 3 /* Line Control Register */
#define MCR 4 /* Modem Control Register */
#define LSR 5 /* Line Status Register */
#define MSR 6 /* Modem Status Register */
#define SR  7 /* Scratch Register */

static u16 serial_base;

#define IOBUFSIZE 8192
static char iobuf[IOBUFSIZE];

void
serial_init(u16 port, u32 baud)
{
  u32 divisor;
  u8 c;

  outb(0x3, port+LCR);
  outb(0x0, port+IER);
  outb(0x0, port+FCR);
  outb(0x3, port+MCR);

  divisor=115200/baud;
  c=inb(port+LCR);
  outb(c|DLAB, port+LCR);
  outb(divisor&0xff, port+DLL);
  outb((divisor>>8)&0xff, port+DLH);
  outb(c&~DLAB, port+LCR);

  serial_base=port;
}

int
serial_putchar(u8 c)
{
  u16 timeout=0xffff;

  while ((inb(serial_base+LSR)&TXRDY)==0
         && --timeout);

  if (0<timeout)
    outb(c, serial_base+THR);

  return 0<timeout?0:-1;
}

int
serial_printk(const char *fmt, ...)
{
  va_list a;
  va_start(a, fmt);
  char *s=iobuf;
  int rv=vsnprintk(iobuf, IOBUFSIZE, fmt, a);

  if (rv)
    while (*s)
      if (0>serial_putchar(*(s++)))
        break;

  va_end(a);
  return rv;
}
