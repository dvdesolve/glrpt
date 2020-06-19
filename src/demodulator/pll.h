/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 3 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details:
 *
 *  http://www.gnu.org/copyleft/gpl.txt
 */

/*****************************************************************************/

#ifndef DEMODULATOR_PLL_H
#define DEMODULATOR_PLL_H

/*****************************************************************************/

#include <complex.h>
#include <stdint.h>

/*****************************************************************************/

typedef enum ModScheme {
    QPSK = 1, /* Standard QPSK */
    DOQPSK,   /* Differential Offset QPSK */
    IDOQPSK   /* Interleaved DOQPSK */
} ModScheme;

typedef struct Costas_t {
    double  nco_phase, nco_freq;
    double  alpha, beta;
    double  damping, bandwidth;
    uint8_t locked;
    double  moving_average;
    ModScheme mode; /* TODO is it actually needed? */
} Costas_t;

/*****************************************************************************/

Costas_t *Costas_Init(double bw, ModScheme mode);
complex double Costas_Mix(Costas_t *self, complex double samp);
void Costas_Correct_Phase(Costas_t *self, double error);
void Costas_Free(Costas_t *self);
double Costas_Delta(complex double sample, complex double cosample);

/*****************************************************************************/

#endif
