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

#include "huffman.h"
#include "../common/shared.h"

int ac_lookup[65536], dc_lookup[65536];

/*------------------------------------------------------------------------*/

  inline int
Get_AC( const uint16_t w )
{
  return( ac_lookup[w] );
}

/*------------------------------------------------------------------------*/

  inline int
Get_DC( const uint16_t w )
{
  return( dc_lookup[w] );
}

/*------------------------------------------------------------------------*/

  static int
Get_AC_Real( const uint16_t w )
{
  int i;

  for( i = 0; (size_t)i < ac_table_len; i++ )
  {
    if( ((w >> (16 - ac_table[i].len)) &
          ac_table[i].mask) == ac_table[i].code )
      return( i );
  }

  return( -1 );
}

/*------------------------------------------------------------------------*/

  static int
Get_DC_Real( const uint16_t w )
{
  if( (w >> 14) == 0 ) return( 0 );

  switch( w >> 13 )
  {
    case 2: return( 1 );
    case 3: return( 2 );
    case 4: return( 3 );
    case 5: return( 4 );
    case 6: return( 5 );
  }

  if( (w  >>  12) == 0x00E ) return( 6 );
  else if( (w  >>  11) == 0x01E )  return( 7 );
  else if( (w  >>  10) == 0x03E )  return( 8 );
  else if( (w  >>   9) == 0x07E )  return( 9 );
  else if( (w  >>   8) == 0x0FE )  return( 10 );
  else if( (w  >>   7) == 0x1FE )  return( 11 );
  else return( -1 );
}

/*------------------------------------------------------------------------*/

  int
Map_Range( const int cat, const int vl )
{
  int maxval, result;
  gboolean sig;

  maxval = (1 << cat) - 1;
  sig = (vl >> (cat - 1)) != 0;
  if( sig ) result = vl;
  else result = vl - maxval;

  return( result );
}

/*------------------------------------------------------------------------*/

  void
Default_Huffman_Table( void )
{
  int k, i, n;
  uint32_t code;
  uint8_t *t;
  int p;
  uint8_t v[65536];
  uint16_t min_code[17], maj_code[17];
  uint16_t max_val, min_val, size_val;
  int min_valn, max_valn, run, size;

  t = t_ac_0;
  p = 16;
  for( k = 1; k <= 16; k++ )
    for( i = 0; i < t[k - 1]; i++ )
    {
      v[(k << 8) + i] = t[p];
      p++;
    }

  code = 0;
  for( k = 1; k <= 16; k++ )
  {
    min_code[k] = (uint16_t)code;
    for( i = 1; i <= t[k - 1]; i++ )
      code++;
    maj_code[k] = (uint16_t)( code - 1 * (uint32_t)(code != 0) );
    code *= 2;
    if( t[k - 1] == 0 )
    {
      min_code[k] = 0xFFFF;
      maj_code[k] = 0;
    }
  }

  ac_table_len = 256;
  mem_alloc( (void **)&ac_table, ac_table_len * sizeof(ac_table_rec_t)) ;

  n = 0;
  min_valn = 1;
  max_valn = 1;
  for( k = 1; k <= 16; k++ )
  {
    min_val = min_code[min_valn];
    max_val = maj_code[max_valn];
    for( i = 0; i < (1 << k); i++ )
    {
      if( ((uint32_t)i <= max_val) &&
          ((uint32_t)i >= min_val) )
      {
        size_val = v[(k << 8) + i - (int)min_val];
        run = size_val >> 4;
        size = size_val & 0x0F;
        ac_table[n].run  = run;
        ac_table[n].size = size;
        ac_table[n].len  = k;
        ac_table[n].mask = (1 << k) - 1;
        ac_table[n].code = (uint32_t)i;
        n++;
      }
    }
    min_valn++;
    max_valn++;
  }

  ac_table_len = (size_t)n;
  mem_realloc( (void **)&ac_table, ac_table_len * sizeof(ac_table_rec_t) );

  for( i = 0; i <= 65535; i++ )
    ac_lookup[i] = Get_AC_Real((uint16_t)i);
  for( i = 0; i <= 65535; i++ )
    dc_lookup[i] = Get_DC_Real((uint16_t)i);
}

/*------------------------------------------------------------------------*/

