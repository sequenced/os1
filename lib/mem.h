#ifndef __MEM_H__
#define __MEM_H__

static inline
void zeromem(void *dst, int len)
{
  while (len--)
    *((char*)dst)=0x0;
}

#endif /* #ifndef __MEM_H__ */
