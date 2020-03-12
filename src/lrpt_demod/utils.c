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

#include "utils.h"
#include "../common/shared.h"

/*------------------------------------------------------------------------*/

/* Clamp_Int8()
 *
 * Clamp a real value to a int8_t
 */
  inline int8_t
Clamp_Int8( double x )
{
  if( x < -128.0 ) return( -128 );
  if( x >  127.0 ) return(  127 );
  if( (x >  0.0) && (x < 1.0) ) return(  1 );
  if( (x > -1.0) && (x < 0.0) ) return( -1 );
  return( (int8_t)x );
} /* Clamp_Int8() */

/*------------------------------------------------------------------------*/

/* Clamp_Double()
 *
 * Clamp a real value in the range [-max_abs, max_abs]
 */
  inline double
Clamp_Double( double x, double max_abs )
{
  if( x >  max_abs ) return(  max_abs );
  if( x < -max_abs ) return( -max_abs );
  return( x );
} /* Clamp_Double() */

/*------------------------------------------------------------------------*/

