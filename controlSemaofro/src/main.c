#include "stm32f1xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include "adcConfig/init_adc.h"
#include "salidasSemaforo/pines_init.h"
#include "uartConfigSemaforo/uart_init.h"

int main(void){
    SystemCoreClockUpdate();
    adc_init();
    adc_task_init();  

    pines_init();
    secuencia_task_init();

    usart1_init();

    vTaskStartScheduler();

    while(1){

    }
}