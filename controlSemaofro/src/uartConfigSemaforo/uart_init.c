#include "uart_init.h"
#include "stm32f1xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


QueueHandle_t xTxQueue = NULL;
QueueHandle_t xRxQueue = NULL;

#define TX_QUEUE_SIZE 128
#define RX_QUEUE_SIZE 128

void UART_TxInit(void)
{
    if (xTxQueue == NULL) {
        xTxQueue = xQueueCreate(TX_QUEUE_SIZE, sizeof(char));
    }
}

void UART_RxInit(void){
       if (xRxQueue == NULL) {
        xRxQueue = xQueueCreate(RX_QUEUE_SIZE, sizeof(char));
    }

}

void usart1_init(void)
{
    // Asegurar que la cola de transmisión esté creada
    UART_TxInit();

    // 1. Habilitar clocks de USART1 (bus APB2) y GPIOA (bus APB2)
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;

    // 2. PA9 (TX1): Alternate function push-pull (50 MHz, CNF=10, MODE=11 en CRH)
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
    GPIOA->CRH |= GPIO_CRH_MODE9 | GPIO_CRH_CNF9_1;

    // 3. PA10 (RX1): Input con pull-up (MODE=00, CNF=10, ODR10=1 en CRH)
    GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
    GPIOA->CRH |= GPIO_CRH_CNF10_1;
    GPIOA->ODR |= GPIO_ODR_ODR10;

    // 4. Configurar Baud Rate (115200 bps en APB2)
    USART1->BRR = SystemCoreClock / 115200;

    // 5. Habilitar TX, RX, interrupción RXNE y USART1
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;

    NVIC_SetPriority(USART1_IRQn, 5); // Configurar prioridad de interrupción
    NVIC_EnableIRQ(USART1_IRQn);       // Habilitar interrupción de USART1
}

void initUart(void)
{
    usart1_init();
}

void usart1_send_char(char c)
{
    if (xTxQueue == NULL) return;

    // Coloca el carácter en la cola
    xQueueSend(xTxQueue, &c, portMAX_DELAY);

    // Habilita interrupción TXE protegida por sección crítica
    taskENTER_CRITICAL();
    USART1->CR1 |= USART_CR1_TXEIE;
    taskEXIT_CRITICAL();
}

void usart1_send_string(const char *str)
{
    if (str == NULL) return;

    while (*str) {
        usart1_send_char(*str++);
    }
}

void USART1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    char received_char;

    // 1. Recepción de datos (RXNE)
    if (USART1->SR & USART_SR_RXNE) {
        received_char = (char)(USART1->DR & 0xFF);

        if (xRxQueue != NULL) {
            xQueueSendFromISR(xRxQueue, &received_char, &xHigherPriorityTaskWoken);
        }
    }

    // 2. Transmisión disponible (TXE)
    if ((USART1->SR & USART_SR_TXE) && (USART1->CR1 & USART_CR1_TXEIE)) {
        char next_char;
        if (xTxQueue != NULL && xQueueReceiveFromISR(xTxQueue, &next_char, &xHigherPriorityTaskWoken) == pdPASS) {
            USART1->DR = (uint8_t)next_char; // Enviar el siguiente carácter
        } else {
            USART1->CR1 &= ~USART_CR1_TXEIE; // Deshabilitar interrupción si no hay más datos
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}