#include "stm8s.h"
#include "stm8s_uart2.h"
#include "retarget.h"
#include <stdio.h>
char uart_putchar(char c)
{
  /* Write a character to the UART2 */
  UART2_SendData8(c);
  /* Loop until the end of transmission */
  while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET);  
}
void printf_uart(char *buf)
{
  uint16_t index=0;
  while(buf[index])
    uart_putchar(buf[index++]);
}
 size_t __write_own(int handle, const unsigned char *buf, size_t bufSize)
{
    size_t chars_written = 0;
    
    /* Ignore flushes */
    if (handle == -1)
    {
      chars_written = (size_t)0; 
    }
    /* Only allow stdout/stderr output */
    else if ((handle != 1) && (handle != 2))
    {
      chars_written = (size_t)-1; 
    }
    /* Parameters OK, call the low-level character output routine */
    else
    {
        while (chars_written < bufSize)
        {
            uart_putchar (buf[chars_written]);
            chars_written++;
        }
    }
    
    return (chars_written);
}
