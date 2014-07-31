#ifndef __ERRNO_H__
#define __ERRNO_H__

#define EINVAL 1 /* Invalid argument */
#define EPERM  2 /* Operation not permitted */

#define errno(n) errno_string_lst[-n]

extern char *errno_string_lst[];

#endif /* #ifndef __ERRNO_H__ */
