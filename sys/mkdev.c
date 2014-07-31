#include <sys/dev.h>
#include <sys/console.h>

int
mkdev(short major, short minor, chrdev *ent)
{
  int rv=0;

  if (DEV_MAJOR_CONSOLE==major)
    rv=mkdev_console(ent);

  return rv;
}
