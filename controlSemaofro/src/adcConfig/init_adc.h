#ifndef INIT_ADC_H
#define INIT_ADC_H

#include "FreeRTOS.h"
#include "queue.h"

/* Cola 1: Medida cruda del ADC (uint16_t) */
extern QueueHandle_t xQueueShuntTension;

/* Cola 2: Medida tras pasar por el filtro digital (float) */
extern QueueHandle_t xQueueShuntFiltrada;

void adc_init(void);
void adc_task_init(void);

#endif