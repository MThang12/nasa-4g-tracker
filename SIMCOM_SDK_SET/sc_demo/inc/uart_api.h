#ifndef __UART_API_H__
#define __UART_API_H__


int UartRead(char *buf, int len, int suspend);
int UartReadLine(char *buf, int maxLen);
int UartReadValue(void);

void sDemo_UartInit(void);


#endif /* __UART_API_H__ */
