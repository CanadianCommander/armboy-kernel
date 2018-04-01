#ifndef UART_H_
#define UART_H_

void initUART(void);

int _write(int file, char *ptr, int len);
int _read (int fd, char *ptr, int len);


#endif
