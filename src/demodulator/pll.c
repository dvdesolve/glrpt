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

#include "pll.h"

#include "../common/common.h"
#include "../common/shared.h"
#include "../glrpt/display.h"
#include "../glrpt/utils.h"
#include "demod.h"

#include <complex.h>
#include <math.h>
#include <stddef.h>

/*****************************************************************************/

/* Costas loop default parameters */
#define FREQ_MAX            0.8     /* Maximum frequency range of locked PLL */
#define COSTAS_DAMP         0.7071  /* 1/M_SQRT2 */
#define COSTAS_INIT_FREQ    0.001
#define AVG_WINSIZE         20000.0 /* My mod, now interp. factor taken into account */
#define DELTA_WINSIZE       100.0   /* Moving Average window for pahase errors */
#define DELTA_WINSIZE_1     99.0    /* Above -1 */
#define LOCKED_WINSIZEX     10.0    /* Error Average window size multiplier (in lock) */
#define ERR_SCALE_QPSK      43.0    /* Scale factors to control magnitude of phase error */
#define ERR_SCALE_DOQPSK    80.0
#define ERR_SCALE_IDOQPSK   80.0
#define LOCKED_BW_REDUCE    4.0     /* PLL Bandwidth reduction (in lock) */
#define LOCKED_ERR_SCALE    10.0    /* Phase error scale on lock */

/*****************************************************************************/

static inline double Clamp_Double(double x, double max_abs);
static void Costas_Recompute_Coeffs(Costas_t *self, double damping, double bw);
static double Lut_Tanh(double val);

/*****************************************************************************/

static double *lut_tanh = NULL;
static double costas_err_scale;

/*****************************************************************************/

/* Clamp_Double()
 *
 * Clamp a double value in the range [-max_abs, max_abs]
 */
static inline double Clamp_Double(double x, double max_abs) {
    if (x > max_abs)
        return max_abs;
    else if (x < -max_abs)
        return -max_abs;
    else
        return x;
}

/*****************************************************************************/

/* Costas_Recompute_Coeffs()
 *
 * Compute the alpha and beta coefficients of the Costas loop from
 * damping and bandwidth, and update them in the Costas object
 */
static void Costas_Recompute_Coeffs(Costas_t *self, double damping, double bw) {
  double denom, bw2;

  bw2 = bw * bw;
  denom = ( 1.0 + 2.0 * damping * bw + bw2 );
  self->alpha = ( 4.0 * damping * bw ) / denom;
  self->beta  = ( 4.0 * bw2 ) / denom;
}

/*****************************************************************************/

/* Lut_Tanh()
 *
 * Reads the tanh table for a given input
 */
static double Lut_Tanh(double val) {
    int ival = (int)val;

    if (ival > 127)
        return 1.0;
    else if (ival < -128)
        return -1.0;
    else
        return lut_tanh[ival + 128];
}

/*****************************************************************************/

/* Costas_Init()
 *
 * Initialize a Costas loop for carrier frequency/phase recovery
 */
Costas_t *Costas_Init(double bw, ModScheme mode) {
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

  /* Error scaling depends on modulation mode */
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
}

/*****************************************************************************/

/* Costas_Mix()
 *
 * Mixes a sample with the PLL nco frequency
 */
complex double Costas_Mix(Costas_t *self, complex double samp) {
  complex double nco_out;
  complex double retval;

  nco_out = cexp( -(complex double)I * self->nco_phase );
  retval = samp * nco_out;
  self->nco_phase += self->nco_freq;
  self->nco_phase  = fmod( self->nco_phase, M_2PI );

  return( retval );
}

/*****************************************************************************/

/* Costas_Correct_Phase()
 *
 * Corrects the phase angle of the Costas PLL
 */
void Costas_Correct_Phase(Costas_t *self, double error) {
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

    /* Report zero signal quality */
    mtd_record.sig_q = 0;
    gtk_entry_set_text( GTK_ENTRY(sig_quality_entry), "0" );
  }

  /* Limit frequency to a sensible range */
  if( (self->nco_freq <= -FREQ_MAX) ||
      (self->nco_freq >= FREQ_MAX) )
    self->nco_freq = 0.0;
}

/*****************************************************************************/

/* Costas_Free()
 *
 * Free the memory associated with the Costas loop object
 */
void Costas_Free(Costas_t *self) {
  free_ptr( (void **)&self );
  free_ptr( (void **)& lut_tanh );
}

/*****************************************************************************/

/* Costas_Delta()
 *
 * Compute the delta phase value to use when
 * correcting the NCO frequency (OQPSK)
 */
double Costas_Delta(complex double sample, complex double cosample) {
  double error;

  error  = ( Lut_Tanh(creal(sample))   * cimag(sample) ) -
           ( Lut_Tanh(cimag(cosample)) * creal(cosample) );
  error /= costas_err_scale;

  return( error );
}
