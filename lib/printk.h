#ifndef __PRINTK_H__
#define __PRINTK_H__

#include <stdarg.h> /* for va_list etc. */

int vsnprintk(char *dst, const int size, const char *fmt, va_list a);
int printk(const char *fmt, ...);

#endif /* #ifndef __PRINTK_H__ */
