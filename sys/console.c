#include <stdarg.h> /* for va_list etc. */
#include <sys/dev.h>
#include <sys/console.h>
#include <sys/errno.h>
#ifdef CONFIG_X86_64
#include <arch/x86_64/vga.h>
#endif

static int close();
static int open(const char *name, int flags);
static int write(const void *buf, int count);
static int read(void *buf, int count);
static int ioctl(int request, ...);

int
mkdev_console(chrdev *ent)
{
  ent->open=open;
  ent->close=close;
  ent->read=read;
  ent->write=write;
  ent->ioctl=ioctl;

  return 0;
}

int
close()
{
  return 0;
}

int
open(const char *name, int flags)
{
  return 0;
}

int
write(const void *buf, int count)
{
  int old;
  int row;
  int col;
  char *s;

  old=count;
  s=(char*)buf;

  while (count--)
    {
#ifdef CONFIG_X86_64
      vga_getpos(&row, &col);

      switch (*s)
        {
        case '\r':
          vga_setpos(row, 0);
          break;

        case '\n':
          if (row<VGA_ROWS-1)
            vga_setpos(row+1, col);
          else
            vga_scroll();
          break;

        default:
          vga_drawc(*s);
          if (col<VGA_COLS-1)
            vga_setpos(row, col+1);
          else if (row<VGA_ROWS-1)
            vga_setpos(row+1, 0);
          else
            {
              vga_setpos(row, 0);
              vga_scroll();
            }
        }
#endif
      s++;
    }

  return (old-count);
}

int
read(void *buf, int count)
{
  return (-EPERM);
}

int
ioctl(int request, ...)
{
  int rv=0;
  va_list a;
  va_start(a, request);

  if (request&DEV_CONSOLE_ADDRESS)
    {
#ifdef CONFIG_X86_64
      vga_setfb((long)va_arg(a, long));
      vga_display_cursor();
#endif
    }
  else if (request&DEV_CONSOLE_CLEAR)
    {
#ifdef CONFIG_X86_64
      vga_clear();
#endif
    }
  else
    rv=(-EINVAL);
  
  va_end(a);

  return rv;
}
