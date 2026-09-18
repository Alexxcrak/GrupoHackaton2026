/* ============================================================
 *  biquad.h - Filtro digital IIR en cascada de biquads
 *  Hackathon 2026 - UTN FRC
 *
 *  Bessel orden 8 | Fc = 4 Hz (norm. en fase) | Fs = 500 Hz
 *  -3 dB real: 2.067 Hz   Asentamiento 1%: 380 ms   Sobrepico: 0.35 %
 *  Coeficientes generados con disenio_filtro.m
 * ============================================================ */
#ifndef BIQUAD_H
#define BIQUAD_H

#define NSEC 4                  /* orden 8 = 4 secciones de 2do orden */

typedef struct {
    float z1[NSEC];
    float z2[NSEC];
} biquad_t;

/* Pone el filtro en cero (arranque en frio: ~380 ms de transitorio). */
void  biquad_reset (biquad_t *f);

/* Arranca el filtro YA asentado en 'val'. Llamar en cada cambio de fase
   con la corriente esperada del color que se enciende. Evita el
   transitorio de encendido y las falsas alarmas que produce.          */
void  biquad_preset(biquad_t *f, float val);

/* Procesa una muestra. Llamar una vez por muestra del ADC (500 Hz). */
float biquad_run   (biquad_t *f, float x);

#endif /* BIQUAD_H */
