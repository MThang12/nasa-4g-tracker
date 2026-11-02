#ifndef __UART_API_H__
#define __UART_API_H__


int demo_uart_read(char *buf, unsigned int size, int timeout_ms);
int demo_uart_readline(char *buf, unsigned int buf_size, int timeout_ms);
int demo_uart_send(const char *buf, unsigned int size);


int demo_uart_init(void);


#endif /* __UART_API_H__ */
