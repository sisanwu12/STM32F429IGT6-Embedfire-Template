#include "stdio.h"
#include "stdlib.h"
#include "sys/stat.h"
#include "unistd.h"
#include <errno.h>

extern int __io_putchar(int ch); /* 可选，用于重定向 printf 输出 */
extern char _end;                /* 来自链接脚本: 堆区起始地址 */
extern char _estack;             /* 来自链接脚本: 栈顶地址 */

static char *heap_end;

/**
 * @brief _sbrk() 为 malloc/new 提供堆内存空间
 */
void *_sbrk(ptrdiff_t incr)
{
  char *prev_heap_end;

  if (heap_end == 0)
    heap_end = &_end;

  prev_heap_end = heap_end;

  // 简单边界检查，防止堆和栈冲突
  if (heap_end + incr > &_estack)
  {
    errno = ENOMEM;
    return (void *)-1;
  }

  heap_end += incr;
  return (void *)prev_heap_end;
}

/**
 * @brief _write() 用于 printf 输出重定向到串口
 */
int _write(int file, char *ptr, int len)
{
  (void)file;
  for (int i = 0; i < len; i++)
  {
    __io_putchar(*ptr++);
  }
  return len;
}

/**
 * @brief 以下函数为标准C库的空实现，以避免链接错误
 */
int _close(int file)
{
  (void)file;
  return -1;
}
int _fstat(int file, struct stat *st)
{
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}
int _isatty(int file)
{
  (void)file;
  return 1;
}
int _lseek(int file, int ptr, int dir)
{
  (void)file;
  (void)ptr;
  (void)dir;
  return 0;
}
int _read(int file, char *ptr, int len)
{
  (void)file;
  (void)ptr;
  (void)len;
  return 0;
}
void _exit(int status)
{
  (void)status;
  for (;;)
  {
  }
}
int _getpid(void) { return 1; }
int _kill(int pid, int sig)
{
  (void)pid;
  (void)sig;
  errno = EINVAL;
  return -1;
}
