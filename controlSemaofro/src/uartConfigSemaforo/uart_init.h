#ifndef UART_INIT_H
#define UART_INIT_H

#include "FreeRTOS.h"
#include "queue.h"

extern QueueHandle_t xTxQueue;
extern QueueHandle_t xRxQueue;

void usart1_init(void);
void initUart(void);
void UART_TxInit(void);
void usart1_send_char(char c);
void usart1_send_string(const char *str);

#endif
