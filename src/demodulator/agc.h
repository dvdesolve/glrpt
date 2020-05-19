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

#ifndef DEMODULATOR_AGC_H
#define DEMODULATOR_AGC_H

/*****************************************************************************/

#include <complex.h>

/*****************************************************************************/

typedef struct Agc_t {
    double average;
    double gain;
    double target_ampl;
    complex double bias;
} Agc_t;

/*****************************************************************************/

Agc_t *Agc_Init(void);
complex double Agc_Apply(Agc_t *self, complex double sample);
void Agc_Free(Agc_t *self);

/*****************************************************************************/

#endif
