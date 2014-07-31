#ifndef __DEV_H__
#define __DEV_H__

#include <sys/limits.h>

struct _chrdev
{
  int perm;
  short major;
  short minor;
  int (*close)();
  int (*open)(const char *name, int flags);
  int (*write)(const void *buf, int count);
  int (*read)(void *buf, int count);
  int (*ioctl)(int request, ...);
};
typedef struct _chrdev chrdev;

int mkdev(short major, short minor, chrdev *ent);

#endif /* #ifndef __DEV_H__ */
