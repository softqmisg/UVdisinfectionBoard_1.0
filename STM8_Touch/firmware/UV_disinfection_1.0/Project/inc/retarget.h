#ifndef __RETARGET__
#define __RETARGET__
#include <stdio.h>
//#define PUTCHAR_PROTOTYPE int extern putchar_me (int c)
//#define GETCHAR_PROTOTYPE int getchar_me (void)
 size_t __write_own(int handle, const unsigned char *buf, size_t bufSize);
void printf_uart(char *buf);
#endif