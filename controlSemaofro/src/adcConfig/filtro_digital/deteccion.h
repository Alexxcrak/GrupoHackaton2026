/* ============================================================
 *  deteccion.h - Deteccion de ramas de LED en falla
 *  Mide la corriente total de la lampara con un shunt comun y
 *  la compara con la esperada para el color comandado.
 * ============================================================ */
#ifndef DETECCION_H
#define DETECCION_H

#include <stdint.h>
#include "biquad.h"

/* ---- Cadena de medicion (ajustar a lo que se monte) -------- */
#define VREF_V        3.3f      /* referencia del ADC            */
#define ADC_CUENTAS   4096.0f   /* 12 bits                       */
#define R_SHUNT_OHM   1.0f      /* shunt en el retorno comun     */
#define GAN_AMP       8.0f      /* ganancia del amplificador     */

/* ---- Lampara: corriente esperada y tamano de rama ----------
 * Topologia 24 V: R/A = 9 LED en serie x 14 ramas
 *                 V   = 6 LED en serie x 21 ramas              */
typedef enum { COL_APAGADO = 0, COL_ROJO, COL_AMARILLO, COL_VERDE, COL_N } color_t;

typedef struct {
    float i_nominal;    /* corriente con la lampara sana  [A] */
    float i_rama;       /* corriente de UNA rama          [A] */
    uint8_t n_ramas;
} lampara_t;

typedef struct {
    biquad_t  filtro;
    color_t   color;
    float     offset_v;       /* auto-cero de la cadena analogica */
    uint16_t  espera;         /* muestras que faltan para asentar */
    float     i_filtrada;
    uint8_t   ramas_caidas;
    uint8_t   valido;
} deteccion_t;

void    det_init      (deteccion_t *d);
void    det_autocero  (deteccion_t *d, uint16_t adc_con_lampara_apagada);
void    det_set_color (deteccion_t *d, color_t c);
void    det_muestra   (deteccion_t *d, uint16_t adc);

float   det_adc_a_amp (const deteccion_t *d, uint16_t adc);
uint8_t det_hay_falla (const deteccion_t *d);

#endif /* DETECCION_H */
