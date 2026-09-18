#include "stm32f1xx.h"
#include "init_adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "filtro_digital/biquad.h"

#define SHUNT_QUEUE_LEN 10

/* Cola 1: Medida cruda del ADC (uint16_t) */
QueueHandle_t xQueueShuntTension = NULL;

/* Cola 2: Medida filtrada (float) */
QueueHandle_t xQueueShuntFiltrada = NULL;

static void vSensorShuntTask(void *pvParameters);
static void vFiltroTask(void *pvParameters);

void adc_init(void){
        //Inicializar los clocks
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_ADC1EN;

    RCC->CFGR &= ~RCC_CFGR_ADCPRE;
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;

    //Configurar PA0 como entrada analogica
    GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);

    //tiempo de muestreo
    ADC1->SMPR2 |= (ADC_SMPR2_SMP0_1 | ADC_SMPR2_SMP0_0);

    ADC1->CR2 |= ADC_CR2_ADON;

        // Demora breve de estabilización (t_STAB >= 1 us)
    for (volatile int i = 0; i < 100; i++);

    // 6. Resetear y ejecutar calibración
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while (ADC1->CR2 & ADC_CR2_RSTCAL);

    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL);

}

static uint16_t adc_read_chanel(void){
    ADC1->SQR3 =  0;
    ADC1->SR  &= ~ADC_SR_EOC;
    ADC1->CR2 |= ADC_CR2_ADON;

    while (!(ADC1->SR & ADC_SR_EOC));
    return (uint16_t)ADC1->DR;

}

void adc_task_init(void){
    if (xQueueShuntTension == NULL) {
        xQueueShuntTension = xQueueCreate(SHUNT_QUEUE_LEN, sizeof(uint16_t));
    }
    if (xQueueShuntFiltrada == NULL) {
        xQueueShuntFiltrada = xQueueCreate(1, sizeof(float));
    }

    xTaskCreate(vSensorShuntTask, "read_shunt", 256, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(vFiltroTask, "filtro_shunt", 256, NULL, tskIDLE_PRIORITY + 2, NULL);
}

static void vSensorShuntTask(void *pvParameters){
    (void) pvParameters;
    uint16_t shunt_tension;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while(1){
        shunt_tension = adc_read_chanel();

        // 1. Mete la medida cruda del ADC a la primera cola
        if (xQueueShuntTension != NULL) {
            xQueueSend(xQueueShuntTension, &shunt_tension, 0);
        }

        // Muestreo a 500 Hz (periodo = 2 ms, definido para el filtro biquad Bessel orden 8)
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2));
    }
}

static void vFiltroTask(void *pvParameters){
    (void) pvParameters;
    biquad_t filtro;
    biquad_reset(&filtro);
    uint16_t raw_val;
    float val_filtrado;

    while(1){
        // 2. Extrae la medida de la cola del ADC
        if (xQueueReceive(xQueueShuntTension, &raw_val, portMAX_DELAY) == pdPASS) {
            // 3. Pasa la medida por el filtro digital de la carpeta
            val_filtrado = biquad_run(&filtro, (float)raw_val);

            // 4. Mete la medida filtrada en la otra cola para su posterior procesamiento
            if (xQueueShuntFiltrada != NULL) {
                xQueueOverwrite(xQueueShuntFiltrada, &val_filtrado);
            }
        }
    }
}