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

#include "alib.h"

#include "viterbi27.h"

#include <glib.h>

#include <stdint.h>
#include <string.h>
#include <strings.h>

/*****************************************************************************/

static uint8_t Byte_Off(uint8_t *l, int n);

/*****************************************************************************/

int Count_Bits(uint32_t n) {
  int result =
    bitcnt[n & 0xFF] +
    bitcnt[(n >>  8) & 0xFF] +
    bitcnt[(n >> 16) & 0xFF] +
    bitcnt[(n >> 24) & 0xFF];

  return( result );
}

/*****************************************************************************/

uint32_t Bio_Peek_n_Bits(bit_io_rec_t *b, const int n) {
  int bit, i, p;
  uint32_t result = 0;

  for( i = 0; i < n; i++ )
  {
    p = b->pos + i;
    bit = ( b->p[p >> 3] >> (7 - (p & 7)) ) & 0x01;
    result = (result << 1) | (uint32_t)bit;
  }

  return( result );
}

/*****************************************************************************/

uint32_t Bio_Fetch_n_Bits(bit_io_rec_t *b, const int n) {
  uint32_t result = Bio_Peek_n_Bits( b, n );
  Bio_Advance_n_Bits( b, n );

  return( result );
}

/*****************************************************************************/

void Bit_Writer_Create(bit_io_rec_t *w, uint8_t *bytes, int len) {
  w->p   = bytes;
  w->len = len;
  w->cur = 0;
  w->pos = 0;
  w->cur_len = 0;
}

/*****************************************************************************/

static uint8_t Byte_Off(uint8_t *l, int n) {
  uint8_t *p = l;
  p += n;
  return( *p );
}

/*****************************************************************************/

void Bio_Write_Bitlist_Reversed(bit_io_rec_t *w, uint8_t *l, int len) {
  uint8_t *bytes;
  int i, byte_index, close_len, full_bytes;
  uint16_t b;

  l = &l[len - 1];
  bytes = w->p;
  byte_index = w->pos;

  if( w->cur_len != 0 )
  {
    close_len = 8 - w->cur_len;
    if( close_len >= len ) close_len = len;

    b = w->cur;
    for( i = 0; i < close_len; i++ )
    {
      b  |= l[0];
      b <<= 1;
      l  -= 1;
    }

    len -= close_len;
    if( (w->cur_len + close_len) == 8 )
    {
      b >>= 1;
      bytes[byte_index] = (uint8_t)b;
      byte_index++;
    }
    else
    {
      w->cur = (uint8_t)b;
      w->cur_len += close_len;
      return;
    }
  }

  full_bytes = len / 8;
  for( i = 0; i < full_bytes; i++ )
  {
    bytes[byte_index] = (uint8_t)
      ( (Byte_Off(l,  0) << 7) |
        (Byte_Off(l, -1) << 6) |
        (Byte_Off(l, -2) << 5) |
        (Byte_Off(l, -3) << 4) |
        (Byte_Off(l, -4) << 3) |
        (Byte_Off(l, -5) << 2) |
        (Byte_Off(l, -6) << 1) |
        Byte_Off(l, -7) );

    byte_index++;
    l -= 8;
  }

  len -= 8 * full_bytes;

  b = 0;
  for( i = 0; i < len; i++ )
  {
    b  |= l[0];
    b <<= 1;
    l--;
  }

  w->cur = (uint8_t)b;
  w->pos = byte_index;
  w->cur_len = len;
}

/*****************************************************************************/

int Ecc_Decode(uint8_t *idata, int pad) {
  int i, j, r, k, deg_lambda, el, deg_omega;
  int syn_error;
  uint8_t q, tmp, num1, num2, den, discr_r;
  uint8_t lambda[33], b[33], reg[33], t[33], omega[33];
  uint8_t root[32], s[32], loc[32];
  uint8_t *data;
  int result = 0;

  data = idata;
  for( i = 0; i < 32; i++ )
    s[i] = data[0];
  for( j = 1; j < 255 - pad; j++ )
    for( i = 0; i < 32; i++ )
      if( s[i] == 0 ) s[i] = data[j];
      else s[i] = data[j] ^ alpha[ (indx[s[i]] + (112 + i) * 11) % 255 ];

  syn_error = 0;
  for( i = 0; i < 32; i++ )
  {
    syn_error |= s[i];
    s[i] = indx[ s[i] ];
  }

  if( syn_error == 0 ) return( 0 );

  bzero( &lambda[1], 32 );
  lambda[0] = 1;

  for( i = 0; i < 33; i++ ) b[i] = indx[ lambda[i] ];
  r  = 0;
  el = 0;

  r++;
  while( r <= 32 )
  {
    discr_r = 0;
    for( i = 0; i < r; i++ )
      if( (lambda[i] != 0) && (s[r - i - 1] != 255) )
        discr_r ^= alpha[ (indx[lambda[i]] + s[r - i - 1]) % 255 ];

    discr_r = indx[discr_r];
    if( discr_r == 255 )
    {
      memmove( &b[1], b, 32 );
      b[0] = 255;
    }
    else
    {
      t[0] = lambda[0];
      for( i = 0; i < 32; i++ )
      {
        if( b[i] != 255 )
          t[i + 1] = lambda[i + 1] ^ alpha[ (discr_r + b[i]) % 255 ];
        else
          t[i + 1] = lambda[i + 1];
      }

      if( 2 * el <= r - 1 )
      {
        el = r - el;
        for( i = 0; i < 32; i++ )
        {
          if( lambda[i] == 0 ) b[i] = 255;
          else b[i] = (uint8_t)( (indx[lambda[i]] - discr_r + 255) % 255 );
        }
      }
      else
      {
        memmove( &b[1], b, 32 );
        b[0] = 255;
      }
      memmove( lambda, t, 33);
    }
    r++;
  }

  deg_lambda = 0;
  for( i = 0; i < 33; i++ )
  {
    lambda[i] = indx[ lambda[i] ];
    if( lambda[i] != 255 ) deg_lambda = i;
  }

  memmove( &reg[1], &lambda[1], 32 );
  result = 0;

  i = 1;
  k = 115;

  while( TRUE )
  {
    if( !(i <= 255) ) break;

    q = 1;
    for( j = deg_lambda; j >= 1; j-- )
    {
      if( reg[j] != 255 )
      {
        reg[j] = (uint8_t)( (reg[j] + j) % 255 );
        q ^= alpha[ reg[j] ];
      }
    }

    if( q != 0 )
    {
      i++;
      k = (k + 116) % 255;
      continue;
    }

    root[result] = (uint8_t)i;
    loc[result]  = (uint8_t)k;
    result++;
    if( result == deg_lambda )
      break;

    i++;
    k = (k + 116) % 255;
  }

  if( deg_lambda != result )
    return( -1 );

  deg_omega = deg_lambda - 1;
  for( i = 0; i <= deg_omega; i++ )
  {
    tmp = 0;
    for( j = i; j >= 0; j-- )
      if( (s[i - j] != 255) && (lambda[j] != 255) )
        tmp ^= alpha[ (s[i - j] + lambda[j]) % 255 ];
    omega[i] = indx[tmp];
  }

  for( j = result - 1; j >= 0; j-- )
  {
    num1 = 0;
    for( i = deg_omega; i >= 0; i-- )
      if( omega[i] != 255 )
        num1 ^= alpha[ (omega[i] + i * root[j]) % 255 ];
    num2 = alpha[ (root[j] * 111 + 255) % 255 ];
    den = 0;

    if( deg_lambda < 31 ) i = deg_lambda;
    else i = 31;
    i &= ~1;

    while( TRUE )
    {
      if( !(i >= 0) ) break;
      if( lambda[i + 1] != 255 )
        den ^= alpha[ (lambda[i + 1] + i * root[j]) % 255 ];
      i -= 2;
    }

    if( (num1 != 0) && (loc[j] >= pad) )
      data[loc[j] - pad] ^= alpha[ (indx[num1] + indx[num2] + 255 - indx[den]) % 255 ];
  }

  return( TRUE );
}

/*****************************************************************************/

void Ecc_Deinterleave(uint8_t *data, uint8_t *output, int pos, int n) {
  int i;
  for( i = 0; i < 255; i++ )
    output[i] = data[ i * n + pos ];
}

/*****************************************************************************/

void Ecc_Interleave(uint8_t *data, uint8_t *output, int pos, int n) {
  int i;
  for( i = 0; i < 255; i++ )
    output[ i * n + pos] = data[i];
}
