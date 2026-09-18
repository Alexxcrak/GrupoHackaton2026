/* ============================================================
 *  tarea_deteccion.c - Tarea FreeRTOS de instrumentacion
 *  Muestrea el shunt a 500 Hz, filtra y detecta ramas caidas.
 *
 *  REQUIERE  configTICK_RATE_HZ = 1000  en FreeRTOSConfig.h
 *  (asi pdMS_TO_TICKS(2) da exactamente 2 ticks = 500 Hz)
 * ============================================================ */
#include "FreeRTOS.h"
#include "task.h"
#include "deteccion.h"

#define PERIODO_MS   2                    /* 500 Hz */

/* --- provisto por el driver de ADC (HAL / libopencm3 / STM32duino) --- */
extern uint16_t adc_leer_canal(uint8_t canal);
#define CANAL_SHUNT_VEHICULAR  0

/* --- provisto por la tarea del semaforo ------------------------------ */
extern color_t semaforo_color_actual(void);

/* --- salida para la tarea de telemetria serie ------------------------ */
volatile float   g_i_lampara   = 0.0f;
volatile uint8_t g_ramas_falla = 0;
volatile uint8_t g_falla       = 0;

void vTareaDeteccion(void *pv)
{
    deteccion_t det;
    color_t     color_prev;
    TickType_t  t_prev;

    (void)pv;

    det_init(&det);

    /* Auto-cero: con todo apagado, lo que lee el ADC es el offset de
       la cadena analogica (TL084 hasta 13 mV = media rama).           */
    vTaskDelay(pdMS_TO_TICKS(50));
    det_autocero(&det, adc_leer_canal(CANAL_SHUNT_VEHICULAR));

    color_prev = semaforo_color_actual();
    det_set_color(&det, color_prev);

    t_prev = xTaskGetTickCount();
    for (;;) {
        color_t color = semaforo_color_actual();

        /* Cambio de fase: precargar el filtro y reabrir la ventana.
           Sin esto, el transitorio de cada cambio dispara falsa alarma. */
        if (color != color_prev) {
            det_set_color(&det, color);
            color_prev = color;
        }

        det_muestra(&det, adc_leer_canal(CANAL_SHUNT_VEHICULAR));

        g_i_lampara   = det.i_filtrada;
        g_ramas_falla = det.ramas_caidas;
        g_falla       = det_hay_falla(&det);

        vTaskDelayUntil(&t_prev, pdMS_TO_TICKS(PERIODO_MS));
    }
}
