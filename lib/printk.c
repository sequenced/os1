#include <sys/errno.h>
#include <printk.h>
#include <mem.h>

#define LONG_MAX_DIGITS 19
#define LONG_MAX_HEX_DIGITS 16
#define ULONG_MAX_DIGITS 20
#define INT_MAX_DIGITS 10
#define INT_MAX_HEX_DIGITS 8
#define UINT_MAX_DIGITS 10

/* conversions */
#define TYPE_DECIMAL  1
#define TYPE_HEX      2
#define TYPE_CHAR     3
#define TYPE_UNSIGNED 4
#define TYPE_STRING   5

/* lengths */
#define FLAG_LONG (1<<0)

static unsigned long power_of_ten[]={1UL,
                                     10UL,
                                     100UL,
                                     1000UL,
                                     10000UL,
                                     100000UL,
                                     1000000UL,
                                     10000000UL,
                                     100000000UL,
                                     1000000000UL,
                                     10000000000UL,
                                     100000000000UL,
                                     1000000000000UL,
                                     10000000000000UL,
                                     100000000000000UL,
                                     1000000000000000UL,
                                     10000000000000000UL,
                                     100000000000000000UL,
                                     1000000000000000000UL,
                                     10000000000000000000UL};

static char hexchar[]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a',
                       'b', 'c', 'd', 'e', 'f'};

static unsigned long bitmask[]={0xdeadbeef,
                                0x000000000000000f,
                                0x00000000000000f0,
                                0x0000000000000f00,
                                0x000000000000f000,
                                0x00000000000f0000,
                                0x0000000000f00000,
                                0x000000000f000000,
                                0x00000000f0000000,
                                0x0000000f00000000,
                                0x000000f000000000,
                                0x00000f0000000000,
                                0x0000f00000000000,
                                0x000f000000000000,
                                0x00f0000000000000,
                                0x0f00000000000000,
                                0xf000000000000000};

struct _conv
{
  int type;
  int flags;
  /* TODO: add width, justification etc. */
};

static int decode_conversion(const char *fmt, struct _conv *sc);
static int convert(char *dst, int remaining, struct _conv *sc, va_list a);
static int string(char *dst, int remaining, const char *s);
static int s64toa(char *buf, long k);
static int u64toa(char *buf, unsigned long k);
static int tohex(char *buf, unsigned long k, int bits);

int
decode_conversion(const char *fmt, struct _conv *sc)
{
  const char *old=fmt;

  zeromem((void*)sc, sizeof(struct _conv));

  /* consume conversion token ('%') */
  fmt++;

  while (*fmt)
    {
      switch (*fmt)
        {
        case 'l':
          sc->flags|=FLAG_LONG;
          break;
        case 'd':
        case 'i':
          sc->type=TYPE_DECIMAL;
          break;
        case 'u':
          sc->type=TYPE_UNSIGNED;
          break;
        case 'x':
          sc->type=TYPE_HEX;
          break;
        case 'c':
          sc->type=TYPE_CHAR;
          break;
        case 's':
          sc->type=TYPE_STRING;
          break;
        default:
          goto out;
        }

      fmt++;
    }

 out:
  return (fmt-old);
}

int
convert(char *dst, int remaining, struct _conv *sc, va_list a)
{
  const char *old=dst;

  switch (sc->type)
    {
    case TYPE_CHAR:
      if (remaining)
        {
          *dst=(char)va_arg(a, int);
          dst++;
        }
      break;

    case TYPE_STRING:
      dst+=string(dst, remaining, va_arg(a, char*));
      break;

    case TYPE_DECIMAL:
      {
        long v;
        int max_len;

        if (FLAG_LONG&sc->flags)
          {
            max_len=LONG_MAX_DIGITS+1+1; /* + '-' + '\0' */
            v=(long)va_arg(a, long);
          }
        else
          {
            max_len=INT_MAX_DIGITS+1+1; /* + '-' + '\0' */
            v=(long)va_arg(a, int);
          }

        if (max_len<=remaining)
          dst+=s64toa(dst, v);
      }
      break;

    case TYPE_UNSIGNED:
      {
        unsigned long v;
        int max_len=max_len=UINT_MAX_DIGITS+1; /* + '\0' */

        if (FLAG_LONG&sc->flags)
          max_len=ULONG_MAX_DIGITS+1; /* + '\0' */

        v=(unsigned long)va_arg(a, unsigned long);

        if (max_len<=remaining)
          dst+=u64toa(dst, v);
      }
      break;

    case TYPE_HEX:
      {
        unsigned long v;
        int max_len=INT_MAX_HEX_DIGITS+1; /* + '\0' */
        int bits=32;

        if (FLAG_LONG&sc->flags)
          {
            max_len=LONG_MAX_HEX_DIGITS+1; /* + '\0' */
            bits=64;
          }

        v=(unsigned long)va_arg(a, unsigned long);

        if (max_len<=remaining)
          dst+=tohex(dst, v, bits);
      }
      break;
    }

  return (dst-old);
}

int
vsnprintk(char *dst, const int size, const char *fmt, va_list a)
{
  int len;
  int remaining=size-1;
  struct _conv conv;

  while (*fmt)
    {
      if ('%'==*fmt)
        {
          len=decode_conversion(fmt, &conv);
          fmt+=len;
          len=convert(dst, remaining, &conv, a);
          dst+=len;
          remaining-=len;
        }
      else
        {
          if (remaining)
            {
              *dst=*fmt;
              fmt++;
              dst++;
              remaining--;
            }
          else
            break;
        }
    }

  *dst='\0';
  return (size-remaining+1);
}

int
u64toa(char *buf, unsigned long k)
{
  const char *old=buf;
  int i=ULONG_MAX_DIGITS;

  /* dealing with zero here avoids special case below */
  if (0UL==k)
    {
      *buf='0';
      return 1;
    }

  while (i--)
    {
      unsigned long rem=k/power_of_ten[i];

      if (0UL==rem)
        {
          if (old!=buf)
            {
              *buf='0';
              buf++;
            }
        }
      else
        {
          *buf=rem+'0';
          buf++;
        }

      k-=(rem*power_of_ten[i]);
    }

  return (buf-old);
}

int
s64toa(char *buf, long k)
{
  const char *old=buf;
  char non_zero=0;
  int i=LONG_MAX_DIGITS;

  if (0L==k)
    {
      *buf='0';
      return 1;
    }

  if (k<0L)
    {
      *buf='-';
      buf++;
      k=-k;
    }

  while (i--)
    {
      long rem=k/power_of_ten[i];

      if (0L==rem)
        {
          /* do not output leading zeros */
          if (non_zero)
            {
              *buf='0';
              buf++;
            }
        }
      else
        {
          *buf=rem+'0';
          buf++;
          non_zero=1;
        }

      k-=(rem*power_of_ten[i]);
    }

  return (buf-old);
}

int
tohex(char *buf, unsigned long k, int bits)
{
  const char *old=buf;
  int i, j;
  for (i=bits/4,j=(bits-4); i>0; i--,j-=4)
    {
      *buf=hexchar[(k&bitmask[i])>>j];
      buf++;
    }

  return (buf-old);
}

int
string(char *dst, int remaining, const char *s)
{
  const char *old=dst;

  while (*s&&remaining--)
    *dst++=*s++;

  return (dst-old);
}

int
printk(const char *fmt, ...)
{
  return (-EPERM);
}
