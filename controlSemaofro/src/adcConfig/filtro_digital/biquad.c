#include "biquad.h"

/* Bessel orden 8, Fc = 4 Hz, Fs = 500 Hz.
 * Formato por fila: { b0, b1, b2, a1, a2 }.  a0 vale 1 y no se guarda.
 * Cada seccion tiene ganancia unitaria en continua: de eso depende
 * que biquad_preset() funcione.                                      */
static const float sos[NSEC][5] = {
    { 5.11852127e-04f, 1.02370425e-03f, 5.11852127e-04f, -1.91052725e+00f, 9.12574658e-01f },
    { 5.44801028e-04f, 1.08960206e-03f, 5.44801028e-04f, -1.91614622e+00f, 9.18325421e-01f },
    { 6.23249628e-04f, 1.24649926e-03f, 6.23249628e-04f, -1.92851190e+00f, 9.31004900e-01f },
    { 7.92064160e-04f, 1.58412832e-03f, 7.92064160e-04f, -1.95145046e+00f, 9.54618715e-01f },
};

void biquad_reset(biquad_t *f)
{
    int s;
    for (s = 0; s < NSEC; s++) { f->z1[s] = 0.0f; f->z2[s] = 0.0f; }
}

void biquad_preset(biquad_t *f, float val)
{
    int s;
    for (s = 0; s < NSEC; s++) {
        const float b1 = sos[s][1], b2 = sos[s][2];
        const float a1 = sos[s][3], a2 = sos[s][4];
        /* Estado estacionario con x = y = val. Se despeja de las
           ecuaciones de la DF-II transpuesta.                      */
        f->z2[s] = (b2 - a2) * val;
        f->z1[s] = (b1 - a1) * val + f->z2[s];
    }
}

float biquad_run(biquad_t *f, float x)
{
    float v = x;
    int s;
    for (s = 0; s < NSEC; s++) {          /* Direct Form II transpuesta */
        const float b0 = sos[s][0], b1 = sos[s][1], b2 = sos[s][2];
        const float a1 = sos[s][3], a2 = sos[s][4];
        const float y = b0 * v + f->z1[s];
        f->z1[s] = b1 * v - a1 * y + f->z2[s];
        f->z2[s] = b2 * v - a2 * y;
        v = y;
    }
    return v;
}
