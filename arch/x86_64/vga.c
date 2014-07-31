#include <arch/x86_64/vga.h>
#include <arch/x86_64/boot.h>
#include <arch/x86_64/types.h>

/* see IBM VGA specification for register and flag details */
#define IOS_BIT              0x1
#define MISC_OUTPUT_REG      0x3cc
#define CRT                  0x3d0
#define ADDR_REG             (CRT|0x4)
#define DATA_REG             (CRT|0x5)
#define CSS_IDX              0xa /* cursor scan line start index */
#define CES_IDX              0xb /* cursor scan line end index */
#define MSL_IDX              0x9 /* maximum scan line index */
#define CLOC_HIGH            0xe
#define CLOC_LOW             0xf

static u16 *fb;

static u8 get_crtc(u8 idx);
static void set_crtc(u8 idx, u8 v);

void
vga_getpos(int *row, int *col)
{
  u16 off;

  off=(get_crtc(CLOC_HIGH)<<8);
  off+=get_crtc(CLOC_LOW);

  *row=off/VGA_COLS;
  *col=off%VGA_COLS;
}

void
vga_setpos(int row, int col)
{
  u16 off;

  off=(row&0xffff)*VGA_COLS+(col&0xffff);

  set_crtc(CLOC_HIGH, off>>8);
  set_crtc(CLOC_LOW, off&0xff);
}

void
vga_scroll()
{
  u16 v;
  int i;

  v=(VGA_DEFAULT_COLOUR<<8|' ');

  for (i=0; i<(VGA_ROWS-1)*VGA_COLS; i++)
    fb[i]=fb[i+VGA_COLS];

  for (;i<VGA_ROWS*VGA_COLS; i++)
    fb[i]=v;
}

void
vga_setfb(long addr)
{
  /* VGA text mode is 16-bit */
  fb=(u16*)addr;
}

void
vga_display_cursor()
{
  u8 mor;
  mor=inb(MISC_OUTPUT_REG);

  if (!(mor&IOS_BIT))
    /* runs on colour/graphics adapter only */
    halt();

  u8 msl=get_crtc(MSL_IDX)&0x1f; /* bits [4:0] are Maximum Scan Line */
  u8 css=get_crtc(CSS_IDX);

  set_crtc(CSS_IDX, css|((msl-1)&0x1f));
  set_crtc(CES_IDX, msl);
}

void
vga_drawc(char c)
{
  int row;
  int col;

  vga_getpos(&row, &col);
  fb[row*VGA_COLS+col]=(VGA_DEFAULT_COLOUR<<8)|c;
}

u8
get_crtc(u8 idx)
{
  outb(idx, ADDR_REG);  /* write index into address register */
  return inb(DATA_REG); /* read data */
}

void
set_crtc(u8 idx, u8 v)
{
  outb(idx, ADDR_REG); /* write index into address register */
  outb(v, DATA_REG);   /* write data */
}

void
vga_clear()
{
  u16 v;
  int i;

  v=(VGA_DEFAULT_COLOUR<<8)|' ';

  for (i=0; i<VGA_COLS*VGA_ROWS; i++)
    {
      fb[i]=v;
    }
}
