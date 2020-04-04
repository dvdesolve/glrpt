/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details:
 *
 *  http://www.gnu.org/copyleft/gpl.txt
 */

/*
 * All data are fixed-point short integers, in which -32768
 * to +32768 represent -1.0 to +1.0 respectively. Integer
 * arithmetic is used for speed, instead of the more natural
 * floating-point.
 *
 * For the forward FFT (time -> freq), fixed scaling is
 * performed to prevent arithmetic overflow, and to map a 0dB
 * sine/cosine wave (i.e. amplitude = 32767) to two -6dB freq
 * coefficients. The return value is always 0.
 *
 * For the inverse FFT (freq -> time), fixed scaling cannot be
 * done, as two 0dB coefficients would sum to a peak amplitude
 * of 64K, overflowing the 32k range of the fixed-point integers.
 * Thus, the fix_fft() routine performs variable scaling, and
 * returns a value which is the number of bits LEFT by which
 * the output must be shifted to get the actual amplitude
 * (i.e. if fix_fft() returns 3, each value of fr[] and fi[]
 * must be multiplied by 8 (2**3) for proper scaling.
 * Clearly, this cannot be done within fixed-point short
 * integers. In practice, if the result is to be used as a
 * filter, the scale_shift can usually be ignored, as the
 * result will be approximately correctly normalized as is.
 * Written by:  Tom Roberts  11/8/89
 * Made portable:  Malcolm Slaney 12/15/94 malcolminterval.com
 * Enhanced:  Dimitrios P. Bouras  14 Jun 2006 dbouras@ieee.org
 *
 * Modified by Neoklis Kyriazis 1 July 2018 nkcyham@yahoo.com
 * The fixed Sinwave table is now dynamically assigned by an
 * initialization function to match FFT size. Function names\
 * have also been changed and the FFT routine now accepts FFT
 * size rather than FFT order and it is modified to some extend.
 */

#include "ifft.h"

#include "../common/shared.h"
#include "../glrpt/callback_func.h"
#include "../glrpt/display.h"
#include "../glrpt/utils.h"

#include <glib.h>

#include <math.h>
#include <stddef.h>
#include <stdint.h>

static int16_t
  ifft_order = 0,
  ifft_width = 0;

static int16_t *Sinewave = NULL;
static char ifft_init = 0;

/*----------------------------------------------------------------------*/

/* Initialize_IFFT()
 *
 * Initializes IFFT() by creating a dynamically allocated
 * Sinewave table and by calculating the FFT order (log2 N)
 */
  gboolean
Initialize_IFFT( int16_t width )
{
  int16_t a, b;
  size_t mreq;
  double w, dw;


  /* Abort if ifft_width is not a power of 2 */
  if( width & (width - 1) )
  {
    Show_Message( "FFT size is not a power of 2", "red" );
    Error_Dialog();
    return( FALSE );
  }

  /* Calculate the order of fft */
  a = width;
  for( ifft_order = 0; a != 1; ifft_order++ )
    a /= 2;

  if( ifft_width != width )
  {
    /* FFT width (size) and data length */
    ifft_width = width;
    ifft_data_length = 2 * (uint16_t)ifft_width;

    /* Allocate the IFFT input data buffer */
    mreq = (size_t)ifft_data_length * sizeof( uint16_t );
    mem_alloc((void **)&ifft_data, mreq );

    /* Allocate the Sine Wave table. This is twice the
     * length needed in IFFT() as it is also used in
     * IFFT_Real() which requires twice the resolution */
    b = ( ifft_data_length * 3 ) / 4;
    mreq = (size_t)b * sizeof( int16_t );
    mem_alloc((void **)&Sinewave, mreq );

    /* Build the Sinewave table */
    dw = 2.0 * M_PI / (double)ifft_data_length;
    w  = 0.0;
    for( a = 0; a < b; a++ )
    {
      Sinewave[a] = (int16_t)( 32767.0 * sin(w) );
      w += dw;
    }

  } /* if( ifft_width != width ) */

  ifft_init = 1;
  return( TRUE );

} /* Initialize_IFFT() */

/*----------------------------------------------------------------------*/

/* Deinit_Ifft()
 *
 * Deinitializes IFFT (frees buffer pointers)
 */
  void
Deinit_Ifft( void )
{
  free_ptr( (void **)&Sinewave );
  free_ptr( (void **)&ifft_data );
  ifft_width = 0;
} /* Deinit_Ifft() */

/*----------------------------------------------------------------------*/

/*
   Imply() - fixed-point multiplication & scaling.
   Scaling ensures that result remains 16-bit.
 */
  static inline int16_t
Imply( int16_t a, int16_t b )
{
  /* Shift right one less bit (i.e. 15-1) */
  int c = ( (int)a * (int)b ) >> 14;

  /* Last bit shifted out = rounding-bit */
  b = c & 0x01;

  /* Last shift + rounding bit */
  a = (int16_t)( c >> 1 ) + b;
  return( a );

} /* Imply() */

/*----------------------------------------------------------------------*/

/* IFFT()
 *
 * This computes an in-place complex-to-complex Radix-2 forward
 * or reverse FFT, BUT in 16-bit (int16_t) integer arithmetic only!
 * reX and imX are the real and imaginary values of 2^m points.
 * ifft_width is the number of points in the time domain as well
 * the number of "bins" returned by the function. The "forward"
 * flag controls the forward or reverse FFT direction.
 */
  void
IFFT( int16_t *data )
{
  int16_t a, b, b2, b3, c, c2, c3, d, e, t, dt;
  int16_t shift, step, dft, bfly;
  int16_t tr, ti, wr, wi, qr, qi;


  /* Decompose time domain signal (bit reversal).
   * This is only needed if the time domain points
   * in reX[] and imX[] were not already decomposed
   * while data was being entered into these buffers */
  a = ifft_width - 1;
  for( b = 1; b < a; b++ )
  {
    c = 0;
    for( d = 0; d < ifft_order; d++ )
    {
      /* Bit reversal of indices */
      c <<= 1;
      c |= (b >> d) & 0x01;
    }

    /* Enter data to locations
     * with bit reversed indices */
    if( b < c )
    {
      b2  = 2 * b;
      c2  = 2 * c;
      b3 = b2 + 1;
      c3 = c2 + 1;

      tr = data[b2];
      ti = data[b3];
      data[b2] = data[c2];
      data[b3] = data[c3];
      data[c2] = tr;
      data[c3] = ti;
    }
  } /* for( b = 1; b < a; b++ ) */

  /* Compute the FFT */
  a = 1;
  e = ifft_order - 1;
  dt = ifft_data_length / 4;

  /* Loop over the number of decomposition
   * (bit reversal) steps (the FFT order) */
  for( step = 0; step < ifft_order; step++ )
  {
    /* Fixed scaling, for proper normalization --
     * there will be log2(n) passes, so this results
     * in an overall factor of 1/n, distributed to
     * maximize arithmetic accuracy. */
    shift = 1;

    /* Loop over sub-ifft's */
    b = a;
    a <<= 1;
    for( dft = 0; dft < b; dft++ )
    {
      /* This works for a Sinewave table which is twice the
       * required length so that it can be used in IFFT_Real() */
      t = (int16_t)( 0x01 << (e - step) ) * dft * 2;
      wr = +Sinewave[t + dt];
      wi = -Sinewave[t];

      /* Maintain scaling to avoid overflow */
      if( shift )
      {
        wr >>= 1;
        wi >>= 1;
      }

      /* Loop over each butterfly and build up the freq domain */
      for( bfly = dft; bfly < ifft_width; bfly += a )
      {
        b2  = 2 * bfly;
        b3 = b2 + 1;
        c2  = 2 * (bfly + b);
        c3 = c2 + 1;

        tr = Imply( wr, data[c2] ) - Imply( wi, data[c3] );
        ti = Imply( wi, data[c2] ) + Imply( wr, data[c3] );
        qr = data[b2];
        qi = data[b3];
        if( shift )
        {
          qr >>= 1;
          qi >>= 1;
        }

        data[c2] = qr - tr;
        data[c3] = qi - ti;
        data[b2] = qr + tr;
        data[b3] = qi + ti;

      } /* for( bfly = a; bfly < ifft_width; bfly += d ) */
    } /* for( dft = 0; dft < e; dft++ ) */
  } /* for( step = 0; step < ifft_order; step++ ) */

} /* IFFT() */

/*----------------------------------------------------------------------*/

/* Compute the forward or inverse Fourier Transform of data, with
   data containing real valued points only. The output is complex
   valued after the first two entries, stored in alternating real
   and imaginary parts. The first two returned entries are the real
   parts of the first and last value from the conjugate symmetric
   output, which are necessarily real. The length must be a power
   of 2.
   "data" The real data stored as alternating real and imaginary
   parts with pairs transposed in time, e.g. data should be an array
   of points in the time domain in sequence X1, X0, X3, X2 ... Xn+1, Xn.
   "forward" TRUE for a forward transform, FALSE for inverse transform
 */
  void
IFFT_Real( int16_t *data )
{
  int16_t tkr, tki, tjr, tji, wr, wi;
  int16_t a, b, c, d, e, f, g, tmp;
  int16_t j, j2, j3, k2, k3, dt;


  /* Do the FFT on data. NOTE that time domain
   * data must have consecutive pairs of points
   * swapped, e.g. X1, X0, X3, X2 ... Xn, Xn-1 */
  IFFT( data );

  /* Carry out the FFT on data */
  g  = ifft_width / 2;
  dt = ifft_data_length / 4;
  for( j = 1; j <= g; j++ )
  {
    k2 = 2 * (ifft_width - j);
    k3 = k2 + 1;
    j2 = 2 * j;
    j3 = j2 + 1;

    tkr = data[k2]; /* Real and imaginary parts of t_k = t_(n/2 - j) */
    tki = data[k3];
    tjr = data[j2]; /* Real and imaginary parts of t_j */
    tji = data[j3];

    wr = Sinewave[j];
    wi = Sinewave[j + dt];

    a = Imply( (tjr - tkr), wr );
    b = Imply( (tji + tki), wi );
    c = Imply( (tjr - tkr), wi );
    d = Imply( (tji + tki), wr );

    e = tjr + tkr;
    f = tji - tki;

    /* Compute entry y[j] */
    data[j2] = ( e + (a + b) ) / 2;
    data[j3] = ( f + (d - c) ) / 2;

    /* Compute entry y[k] */
    data[k2] = ( e - (b + a) ) / 2;
    data[k3] = ( (d - c) - f ) / 2;

  } /* for( j = 1; j <= length / 4; ++j ) */

  /* Compute final y0 and y_{N/2}, store in data[0], data[1] */
  tmp      = data[0];
  data[0] += data[1];
  data[1]  = tmp - data[1];

} /* IFFT_Real() */

/*----------------------------------------------------------------------*/

/* IFFT_Data()
 *
 * Collects signal samples and does an IFFT when
 * enough samples are collected for the Waterfall Display
 */
  void
IFFT_Data( short sample )
{
  static int data = 0;

  static uint
    idx  = 0, /* ifft input buffer idx */
         cnt  = 0; /* Count of calls to this function */

  if( ! ifft_init ) return;

  /* Summate (decimate) samples for the IFFT */
  data += sample;

  /* Reset stride (decimation) counter */
  if( ++cnt >= rc_data.ifft_decimate )
  {
    /* Normalize IFFT input samples */
    data /= rc_data.ifft_decimate;

    /* Place data in swapped order, X1, X0, X3, X2 ... Xn+1, Xn
     * so that IFFT_Real() works properly with real data input */
    if( idx & 0x01 )
      ifft_data[idx - 1] = (int16_t)data;
    else
      ifft_data[idx + 1] = (int16_t)data;
    idx++;

    /* Display waterfall when input buffer full */
    if( idx >= ifft_data_length )
    {
      Display_Waterfall();
      idx = 0;
    }

    /* Clear for next summation */
    data = 0;
    cnt = 0;
  } /* if( ++cnt >= rc_data.ifft_stride ) */

} /* IFFT_Data() */
