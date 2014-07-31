#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <sys/dev.h>

#define DEV_MAJOR_CONSOLE 0

#define DEV_CONSOLE_ADDRESS (1<<0)
#define DEV_CONSOLE_CLEAR   (1<<1)

int mkdev_console(chrdev *ent);

#endif /* #ifndef __CONSOLE_H__ */
