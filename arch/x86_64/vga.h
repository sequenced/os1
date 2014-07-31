#ifndef __VGA_H__
#define __VGA_H__

#include <arch/x86_64/types.h>

#define VGA_COLS 80
#define VGA_ROWS 25
#define VGA_DEFAULT_COLOUR 0x2a

void vga_getpos(int *row, int *col);
void vga_setpos(int row, int col);
void vga_scroll();
void vga_setfb(long addr);
void vga_display_cursor();
void vga_drawc(char c);
void vga_clear();

#endif /* #ifndef __VGA_H__ */
