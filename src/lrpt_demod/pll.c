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

#include "pll.h"

#include "../common/common.h"
#include "../common/shared.h"
#include "demod.h"
#include "../glrpt/display.h"
#include "../glrpt/utils.h"
#include "utils.h"

#include <complex.h>
#include <math.h>
#include <stddef.h>

static double *lut_tanh = NULL;
static double costas_err_scale;

/*------------------------------------------------------------------------*/

/* Costas_Recompute_Coeffs()
 *
 * Compute the alpha and beta coefficients of the Costas loop from
 * damping and bandwidth, and update them in the Costas object
 */
  void
Costas_Recompute_Coeffs( Costas_t *self, double damping, double bw )
{
  double denom, bw2;

  bw2 = bw * bw;
  denom = ( 1.0 + 2.0 * damping * bw + bw2 );
  self->alpha = ( 4.0 * damping * bw ) / denom;
  self->beta  = ( 4.0 * bw2 ) / denom;
} /* Costas_Recompute_Coeffs() */

/* Lut_Tanh()
 *
 * Reads the tanh table for a given input
 */
  static double
Lut_Tanh( double val )
{
  int ival = (int)val;
  if( ival >  127 ) return(  1.0 );
  if( ival < -128 ) return( -1.0 );
  return( lut_tanh[ival + 128] );
} /* Lut_Tanh() */

/*------------------------------------------------------------------------*/

/* Costas_Init()
 *
 * Initialize a Costas loop for carrier frequency/phase recovery
 */
  Costas_t *
Costas_Init( double bw, ModScheme mode )
{
  int idx;
  Costas_t *costas = NULL;

  mem_alloc( (void **)&costas, sizeof(*costas) );

  costas->nco_freq  = COSTAS_INIT_FREQ;
  costas->nco_phase = 0.0;

  Costas_Recompute_Coeffs( costas, COSTAS_DAMP, bw );

  costas->damping   = COSTAS_DAMP;
  costas->bandwidth = bw;
  costas->mode      = mode;

  /* My mod, to allow reset of avg_winsize in Costas_Resync()
   * if Receiving is stopped and restarted while PLL is locked */
  costas->locked = 1;

  /* Huge but needed to stop stray locks at startup */
  costas->moving_average = 1000000.0;

  /* Error sacling depends on modulation mode */
  switch( mode )
  {
    case QPSK:
    costas_err_scale = ERR_SCALE_QPSK;
    break;

    case DOQPSK:
    costas_err_scale = ERR_SCALE_DOQPSK;
    break;

    case IDOQPSK:
    costas_err_scale = ERR_SCALE_IDOQPSK;
    break;
  }

  mem_alloc( (void **)&lut_tanh, sizeof(double) * 256 );
  for( idx = 0; idx < 256; idx++ )
    lut_tanh[idx] = tanh( (double)(idx - 128) );

  return( costas );
} /* Costas_Init() */

/*------------------------------------------------------------------------*/

/* Costas_Mix()
 *
 * Mixes a sample with the PLL nco frequency
 */
  complex double
Costas_Mix( Costas_t *self, complex double samp )
{
  complex double nco_out;
  complex double retval;

  nco_out = cexp( -(complex double)I * self->nco_phase );
  retval  = samp * nco_out;
  self->nco_phase += self->nco_freq;
  self->nco_phase  = fmod( self->nco_phase, M_2PI );

  return( retval );
} /* Costas_Mix() */

/*------------------------------------------------------------------------*/

/* Costas_Correct_Phase()
 *
 * Corrects the phase angle of the Costas PLL
 */
  void
Costas_Correct_Phase( Costas_t *self, double error )
{
  static double avg_winsize   = AVG_WINSIZE;
  static double avg_winsize_1 = AVG_WINSIZE - 1.0;
  static double delta = 0.0; /* Average phase error */

  error = Clamp_Double( error, 1.0 );

  self->moving_average *= avg_winsize_1;
  self->moving_average += fabs( error );
  self->moving_average /= avg_winsize;

  self->nco_phase += self->alpha * error;
  self->nco_phase  = fmod( self->nco_phase, M_2PI );

  /* Calculate sliding window average of phase error */
  if( self->locked ) error /= LOCKED_ERR_SCALE;
  delta *= DELTA_WINSIZE_1;
  delta += self->beta * error;
  delta /= DELTA_WINSIZE;
  self->nco_freq += delta;

  /* Detect whether the PLL is locked, and decrease the BW if it is */
  if( !self->locked &&
      (self->moving_average < rc_data.pll_locked) )
  {
    Costas_Recompute_Coeffs(
        self, self->damping, self->bandwidth / LOCKED_BW_REDUCE );
    self->locked  = 1;
    avg_winsize   = AVG_WINSIZE * LOCKED_WINSIZEX / (double)rc_data.interp_factor;
    avg_winsize_1 = avg_winsize - 1.0;

    Display_Icon( pll_lock_icon, "gtk-yes" );
  }
  else if( self->locked &&
      (self->moving_average > rc_data.pll_unlocked) )
  {
    Costas_Recompute_Coeffs( self, self->damping, self->bandwidth );
    self->locked  = 0;
    avg_winsize   = AVG_WINSIZE / (double)rc_data.interp_factor;
    avg_winsize_1 = avg_winsize - 1.0;

    Display_Icon( pll_lock_icon, "gtk-no" );
    Display_Icon( frame_icon, "gtk-no" );
  }

  /* Limit frequency to a sensible range */
  if( (self->nco_freq <= -FREQ_MAX) ||
      (self->nco_freq >= FREQ_MAX) )
    self->nco_freq = 0.0;

} /* Costas_Correct_Phase() */


/*------------------------------------------------------------------------*/

/* Costas_Free()
 *
 * Free the memory associated with the Costas loop object
 */
  void
Costas_Free( Costas_t *self )
{
  free_ptr( (void **)&self );
  free_ptr( (void **)& lut_tanh );
} /* Costas_Free() */

/*------------------------------------------------------------------------*/

/* Costas_Delta()
 *
 * Compute the delta phase value to use when
 * correcting the NCO frequency (OQPSK)
 */
  double
Costas_Delta( complex double sample, complex double cosample )
{
  double error;

  error  = ( Lut_Tanh(creal(sample))   * cimag(sample) ) -
           ( Lut_Tanh(cimag(cosample)) * creal(cosample) );
  error /= costas_err_scale;

  return( error );
} /* Costas_Delta() */
