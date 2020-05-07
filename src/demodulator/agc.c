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

#include "agc.h"

#include "../glrpt/utils.h"
#include "demod.h"

#include <complex.h>
#include <math.h>
#include <stddef.h>

/*****************************************************************************/

/* AGC default parameters */
#define AGC_WINSIZE         65536.0  // 1024*64
#define AGC_WINSIZE_1       65535.0  // 1024*64
#define AGC_TARGET          180.0
#define AGC_MAX_GAIN        20.0
#define AGC_BIAS_WINSIZE    262144.0 // 256*1024
#define AGC_BIAS_WINSIZE_1  262143.0 // 256*1024 - 1

/*****************************************************************************/

/* Agc_Init()
 *
 * Initialize an AGC object
 */
Agc_t *Agc_Init(void) {
  Agc_t *agc = NULL;

  mem_alloc( (void **)&agc, sizeof(*agc) );

  agc->target_ampl = AGC_TARGET;
  agc->average     = AGC_TARGET;
  agc->gain        = 1.0;
  agc->bias        = 0.0;

  return( agc );
}

/*****************************************************************************/

/* Agc_Apply()
 *
 * Apply the right gain to a sample
 */
complex double Agc_Apply(Agc_t *self, complex double sample) {
  double rho;

  /* Sliding window average */
  self->bias *= AGC_BIAS_WINSIZE_1;
  self->bias += sample;
  self->bias /= AGC_BIAS_WINSIZE;
  sample     -= self->bias;

  /* Update the sample magnitude average */
  double real = creal( sample );
  double imag = cimag( sample );
  rho = sqrt( real * real + imag * imag );
  self->average *= AGC_WINSIZE_1;
  self->average += rho;
  self->average /= AGC_WINSIZE;

  /* Apply AGC to samples */
  self->gain = self->target_ampl / self->average;
  if( self->gain > AGC_MAX_GAIN )
    self->gain = AGC_MAX_GAIN;

  return( sample * self->gain );
}

/*****************************************************************************/

/* Agc_Free()
 *
 * Free an AGC object
 */
void Agc_Free(Agc_t *self) {
  free_ptr( (void **)&self );
}
