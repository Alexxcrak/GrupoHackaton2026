#include "deteccion.h"

/* Muestras a esperar tras un cambio de fase antes de dar por valida
   la medicion: 380 ms de asentamiento del filtro + margen, a 500 Hz. */
#define MUESTRAS_ASENTAMIENTO  225   /* 450 ms */

/* Umbral: se declara rama caida a partir de media rama de desviacion,
   asi el redondeo a cantidad de ramas es inequivoco.                 */
#define FRACCION_UMBRAL        0.5f

static const lampara_t LAMPARA[COL_N] = {
    /* i_nominal, i_rama, n_ramas */
    { 0.000f, 0.000f,  0 },   /* COL_APAGADO  */
    { 0.350f, 0.025f, 14 },   /* COL_ROJO     */
    { 0.378f, 0.027f, 14 },   /* COL_AMARILLO */
    { 0.315f, 0.015f, 21 },   /* COL_VERDE    */
};

float det_adc_a_amp(const deteccion_t *d, uint16_t adc)
{
    const float v = ((float)adc * VREF_V / ADC_CUENTAS) - d->offset_v;
    return v / (GAN_AMP * R_SHUNT_OHM);
}

void det_init(deteccion_t *d)
{
    biquad_reset(&d->filtro);
    d->color        = COL_APAGADO;
    d->offset_v     = 0.0f;
    d->espera       = MUESTRAS_ASENTAMIENTO;
    d->i_filtrada   = 0.0f;
    d->ramas_caidas = 0;
    d->valido       = 0;
}

/* Cancela el offset del TL084 (hasta 13 mV, = media rama).
   Llamar al arranque con todas las lamparas apagadas.        */
void det_autocero(deteccion_t *d, uint16_t adc_con_lampara_apagada)
{
    d->offset_v = (float)adc_con_lampara_apagada * VREF_V / ADC_CUENTAS;
}

/* Llamar en CADA cambio de fase. Precarga el filtro en la corriente
   esperada del color nuevo y abre la ventana de asentamiento.       */
void det_set_color(deteccion_t *d, color_t c)
{
    if (c >= COL_N) c = COL_APAGADO;
    d->color  = c;
    d->valido = 0;
    d->espera = MUESTRAS_ASENTAMIENTO;
    biquad_preset(&d->filtro, LAMPARA[c].i_nominal);
}

void det_muestra(deteccion_t *d, uint16_t adc)
{
    const lampara_t *L = &LAMPARA[d->color];

    d->i_filtrada = biquad_run(&d->filtro, det_adc_a_amp(d, adc));

    if (d->espera) { d->espera--; return; }   /* todavia asentando */

    d->valido = 1;

    if (L->n_ramas == 0) { d->ramas_caidas = 0; return; }

    {
        const float falta = L->i_nominal - d->i_filtrada;
        if (falta < L->i_rama * FRACCION_UMBRAL) {
            d->ramas_caidas = 0;
        } else {
            int n = (int)((falta / L->i_rama) + 0.5f);   /* redondeo */
            if (n > L->n_ramas) n = L->n_ramas;
            d->ramas_caidas = (uint8_t)n;
        }
    }
}

uint8_t det_hay_falla(const deteccion_t *d)
{
    return (d->valido && d->ramas_caidas > 0);
}
