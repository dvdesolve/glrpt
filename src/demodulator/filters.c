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

#include "filters.h"

#include "../glrpt/utils.h"
#include "demod.h"

#include <complex.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/*****************************************************************************/

static double Compute_RRC_Coeff(
        int stage_no,
        uint32_t taps,
        double osf,
        double alpha);
static Filter_t *Filter_New(uint32_t fwd_count, double *fwd_coeff);

/*****************************************************************************/

/* Compute_RRC_Coeff()
 *
 * Variable alpha RRC filter coefficients
 * Taken from https://www.michael-joost.de/rrcfilter.pdf
 */
static double Compute_RRC_Coeff(
        int stage_no,
        uint32_t taps,
        double osf,
        double alpha) {
  double coeff;
  double t, mpt, at4;
  double interm;
  int    order;

  order = ( (int)taps - 1 ) / 2;

  /* Handle the 0/0 case */
  if( order == stage_no )
    return( 1.0 - alpha + 4.0 * alpha / M_PI );

  t = (double)( abs(order - stage_no) ) / osf;
  mpt = M_PI * t;
  at4 = 4.0 * alpha * t;
  coeff = sin( mpt * (1.0 - alpha) ) + at4 * cos( mpt * (1.0 + alpha) );
  interm = mpt * ( 1.0 - at4 * at4 );

  return( coeff / interm );
}

/*****************************************************************************/

/* Filter_New()
 *
 * Create a new filter, an FIR if back_count is 0, an IIR filter
 * otherwise. Variable length arguments are two ptrs to doubles,
 * holding the coefficients to use in the filter
 */
static Filter_t *Filter_New(uint32_t fwd_count, double *fwd_coeff) {
  Filter_t *flt = NULL;
  uint32_t idx;

  mem_alloc( (void **)&flt, sizeof(*flt) );
  flt->fwd_count = fwd_count;
  flt->fwd_coeff = NULL;

  if( fwd_count )
  {
    /* Initialize the filter memory nodes and forward coefficients */
    mem_alloc( (void **)&(flt->fwd_coeff), sizeof(*flt->fwd_coeff) * fwd_count );
    flt->memory = calloc( sizeof(*flt->memory), fwd_count );
    for( idx = 0; idx < fwd_count; idx++ )
      flt->fwd_coeff[idx] = (double)fwd_coeff[idx];
  }

  return( flt );
}

/*****************************************************************************/

/* Filter_RRC()
 *
 * Create a RRC (root raised cosine) filter
 */
Filter_t *Filter_RRC(
        uint32_t order,
        uint32_t factor,
        double osf,
        double alpha) {
  uint32_t idx;
  uint32_t taps;
  double  *coeffs = NULL;
  Filter_t *rrc;

  taps = order * 2 + 1;
  mem_alloc( (void **)&coeffs, sizeof(*coeffs) * taps );

  /* Compute the filter coefficients */
  for( idx = 0; idx < taps; idx++ )
    coeffs[idx] = Compute_RRC_Coeff( (int)idx, taps, osf * (double)factor, alpha );

  rrc = Filter_New( taps, coeffs );
  free_ptr( (void **)&coeffs );

  return( rrc );
}

/*****************************************************************************/

/* Filter_Fwd()
 *
 * Feed a signal through a filter, and output the result
 */
complex double Filter_Fwd(Filter_t *const self, complex double in) {
  uint32_t idc;       /* Coefficients index */
  static int idm = 0; /* Ring buiffer (memory) index */
  complex double out;

  /* Update the memory nodes, save input to first node */
  self->memory[idm] = in;

  /* Calculate the feed-forward output */
  out = 0.0;
  idc = 0;

  /* Summate nodes till the end of the ring buffer */
  while( idm < (int)self->fwd_count )
    out += self->memory[idm++] * self->fwd_coeff[idc++];

  /* Go back to the beginning of the ring
   * buffer and summate remaining nodes */
  idm = 0;
  while( idc < self->fwd_count )
    out += self->memory[idm++] * self->fwd_coeff[idc++];

  /* Move back (left) in the ring buffer */
  idm--;
  if( idm < 0 ) idm += self->fwd_count;

  return( out );

  /****** Original code
  size_t idx, fc_1;
  complex double out;

  Update the memory nodes
  fc_1 = self->fwd_count - 1;
  memmove( self->memory + 1, self->memory, sizeof(*self->memory) * fc_1 );
  self->memory[0] = in;

  Calculate the feed-forward output
  out = in * self->fwd_coeff[0];
  for( idx = fc_1; idx > 0; idx-- )
    out += self->memory[idx] * self->fwd_coeff[idx];
  return( out );
  *********/
}

/*****************************************************************************/

/* Filter_Free()
 *
 * Free a filter object
 */
void Filter_Free(Filter_t *self) {
  if( self->memory )
    free_ptr( (void **)&(self->memory) );
  if( self->fwd_count )
    free_ptr( (void **)&(self->fwd_coeff) );

  free_ptr( (void **)&self );
}
