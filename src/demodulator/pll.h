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

#include "demod.h"

#include <complex.h>

/*****************************************************************************/

Costas_t *Costas_Init(double bw, ModScheme mode);
complex double Costas_Mix(Costas_t *self, complex double samp);
void Costas_Correct_Phase(Costas_t *self, double error);
void Costas_Free(Costas_t *self);
double Costas_Delta(complex double sample, complex double cosample);

/*****************************************************************************/

#endif
