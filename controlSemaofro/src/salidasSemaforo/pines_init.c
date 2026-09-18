#include "stm32f1xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "uartConfigSemaforo/uart_init.h"

extern QueueHandle_t xQueueShuntFiltrada;

/* Constantes para la conversion ADC a Tension */
#define VREF_V          3.3f
#define ADC_CUENTAS     4096.0f

/* Umbrales de tension (en Voltios) para validar el funcionamiento */
#define TENSION_MIN_V   0.5f    /* Umbral minimo en voltios (ajustable) */
#define TENSION_MAX_V   3.0f    /* Umbral maximo en voltios (ajustable) */

static void vSecuenciaTask(void *pvParameters);
static void vSecuenciaTask2(void *pvParameters);

//n permite controlar el pin que se prende pudiendolo expandir a los semaforos necesarios.
static inline void led_on(uint8_t n)     { GPIOA->BSRR = (1U << n); }
static inline void led_off(uint8_t n)    { GPIOA->BRR  = (1U << n); }

void pines_init(void){
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // Configuración Salida Push-Pull 2MHz:
    // Semáforo 1: p1 (Verde), p2 (Amarillo), p3 (Rojo)
    // Semáforo 2: p4 (Verde), p5 (Amarillo), p6 (Rojo)
    GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1 |
                    GPIO_CRL_MODE2 | GPIO_CRL_CNF2 |
                    GPIO_CRL_MODE3 | GPIO_CRL_CNF3 |
                    GPIO_CRL_MODE4 | GPIO_CRL_CNF4 |
                    GPIO_CRL_MODE5 | GPIO_CRL_CNF5 |
                    GPIO_CRL_MODE6 | GPIO_CRL_CNF6);

    GPIOA->CRL |=  (GPIO_CRL_MODE1_1 | GPIO_CRL_MODE2_1 | GPIO_CRL_MODE3_1 |
                    GPIO_CRL_MODE4_1 | GPIO_CRL_MODE5_1 | GPIO_CRL_MODE6_1);

    // Arrancan todos apagados
    led_off(1);
    led_off(2);
    led_off(3);
    led_off(4);
    led_off(5);
    led_off(6);
}

void secuencia_task_init(void){
    xTaskCreate(vSecuenciaTask, "secuencia_semaforo_1", 256, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vSecuenciaTask2, "secuencia_semaforo_2", 256, NULL, tskIDLE_PRIORITY, NULL);
}

static void vSecuenciaTask(void *pvParameters){
    (void)pvParameters;
    TickType_t xLastWake = xTaskGetTickCount();

    float shunt_filtrada;
    float tension;

    while(1){
        // Recibe la lectura filtrada del ADC
        //xQueueReceive(xQueueShuntFiltrada, &shunt_filtrada, portMAX_DELAY);
            // Conversion de cuentas del ADC a tension (Voltios)
           // tension = shunt_filtrada * (VREF_V / ADC_CUENTAS);

            // Si la tension esta entre 2 valores la secuencia corre
            

            // Semaforo 1
                led_on(3);                                          
                vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(2000));
                led_off(3);

                led_on(1);                                          
                vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(33000));
                led_off(1);
                
                led_on(2);    
                vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(5000));
                led_off(2);    

                led_on(3);                                          
                vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(40000));
                led_off(3);
    
        
    }
}

static void vSecuenciaTask2(void *pvParameters){
    (void)pvParameters;
    TickType_t xLastWake = xTaskGetTickCount();

    while(1){
        // Semaforo 2: Pin 4 (Verde), Pin 5 (Amarillo), Pin 6 (Rojo)
        // 1. Rojo prendido 42 seg
        led_on(6);
        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(42000));
        led_off(6);

        // 2. Verde prendido de 42 a 75 seg (33 seg)
        led_on(4);
        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(33000));
        led_off(4);

        // 3. Amarillo prendido de 75 a 80 seg (5 seg)
        led_on(5);
        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(5000));
        led_off(5);
    }
}