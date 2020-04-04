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

#include "dct.h"

#include <glib.h>

#include <math.h>

/*------------------------------------------------------------------------*/

static double cosine[8][8];
static double alpha[8];

  static void
Init_Cos( void )
{
  static gboolean cos_inited = FALSE;
  int x, y;

  if( cos_inited ) return;
  cos_inited = TRUE;

  for( y = 0; y <= 7; y++ )
    for( x = 0; x <= 7; x++ )
      cosine[y][x] = cos( M_PI / 16.0 * (2.0 * (double)y + 1.0) * (double)x );

  alpha[0] = 1.0 / sqrt( 2.0 );
  for( x = 1; x <= 7; x++ )
    alpha[x] = 1.0;
}

/*------------------------------------------------------------------------*/

  void
Flt_Idct_8x8( double *res, const double *inpt  )
{
  int x, y, u;
  double s, cxu;

  Init_Cos();

  for( y = 0; y <= 7; y++ )
    for( x = 0; x <= 7; x++ )
    {
      s = 0;
      //for u = 0 to 7 do for v = 0 to 7 do
      //s = s+inp[v*8+u]*alpha[u]*alpha[v]*cosine[x][u]*cosine[y][v];
      for( u = 0; u <= 7; u++ )
      {
        cxu = alpha[u] * cosine[x][u];
        //Unrolled to 8
        s += cxu * (
            inpt[0 * 8 + u] * alpha[0] * cosine[y][0] +
            inpt[1 * 8 + u] * alpha[1] * cosine[y][1] +
            inpt[2 * 8 + u] * alpha[2] * cosine[y][2] +
            inpt[3 * 8 + u] * alpha[3] * cosine[y][3] +
            inpt[4 * 8 + u] * alpha[4] * cosine[y][4] +
            inpt[5 * 8 + u] * alpha[5] * cosine[y][5] +
            inpt[6 * 8 + u] * alpha[6] * cosine[y][6] +
            inpt[7 * 8 + u] * alpha[7] * cosine[y][7] );
      }
      res[y * 8 + x] = s / 4.0;
    }
}
