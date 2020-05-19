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

#include "correlator.h"

#include <stdint.h>
#include <strings.h>

/*****************************************************************************/

#define CORR_LIMIT  55

/*****************************************************************************/

static uint8_t Rotate_IQ(uint8_t data, int shift);
static uint64_t Rotate_IQ_QW(uint64_t data, int shift);
static uint64_t Flip_IQ_QW(uint64_t data);
static void Corr_Set_Patt(corr_rec_t *c, int n, uint64_t p);
static void Corr_Reset(corr_rec_t *c);

/*****************************************************************************/

static uint8_t rotate_iq_tab[256];
static uint8_t invert_iq_tab[256];
static int corr_tab[256][256];

/*****************************************************************************/

int Hard_Correlate(const uint8_t d, const uint8_t w) {
    return corr_tab[d][w];
}

/*****************************************************************************/

void Init_Correlator_Tables(void) {
  int i, j;

  for( i = 0; i <= 255; i++ )
  {
    rotate_iq_tab[i] = (uint8_t)( (((i & 0x55) ^ 0x55) << 1) | ((i & 0xAA) >> 1) );
    invert_iq_tab[i] = (uint8_t)( ( (i & 0x55)         << 1) | ((i & 0xAA) >> 1) );

    //Correlation between a soft sample i and a hard value j
    for( j = 0; j <= 255; j++ )
      corr_tab[i][j] =
        (int)( ((i > 127) && (j == 0)) || ((i <= 127) && (j == 255)) );
  }
}

/*****************************************************************************/

static uint8_t Rotate_IQ(uint8_t data, int shift) {
  uint8_t result = data;
  if( (shift == 1) | (shift == 3) )
    result = rotate_iq_tab[result];

  if( (shift == 2) | (shift == 3) )
    result = result ^ 0xFF;

  return( result );
}

/*****************************************************************************/

static uint64_t Rotate_IQ_QW(uint64_t data, int shift) {
  int i;
  uint64_t result = 0;
  uint8_t  bdata;

  for( i = 0; i < PATTERN_CNT; i++ )
  {
    bdata = (uint8_t)( (data >> (56 - 8 * i)) & 0xff );
    result <<= 8;
    result |= Rotate_IQ( bdata, shift );
  }

  return( result );
}

/*****************************************************************************/

static uint64_t Flip_IQ_QW(uint64_t data) {
  int i;
  uint64_t result = 0;
  uint8_t  bdata;

  for( i = 0; i < PATTERN_CNT; i++ )
  {
    bdata = (uint8_t)( (data >> (56 - 8 * i)) & 0xff );
    result <<= 8;
    result |= invert_iq_tab[bdata];
  }

  return( result );
}

/*****************************************************************************/

void Fix_Packet(void *data, int len, int shift) {
  int j;
  int8_t *d;
  int8_t b;

  d = (int8_t *)data;
  switch( shift )
  {
    case 4:
      for( j = 0; j < len / 2; j++ )
      {
        b = d[j * 2 + 0];
        d[j * 2 + 0] = d[j * 2 + 1];
        d[j * 2 + 1] = b;
      }
      break;

    case 5:
      for( j = 0; j < len / 2; j++ )
      {
        d[j * 2 + 0] = -d[j * 2 + 0];
      }
      break;

    case 6:
      for( j = 0; j < len / 2; j++ )
      {
        b = d[ j * 2 + 0];
        d[j * 2 + 0] = -d[j * 2 + 1];
        d[j * 2 + 1] = -b;
      }
      break;

    case 7:
      for( j = 0; j < len / 2; j++ )
      {
        d[j * 2 + 1] = -d[j * 2 + 1];
      }
  }
}

/*****************************************************************************/

static void Corr_Set_Patt(corr_rec_t *c, int n, uint64_t p) {
  int i;

  for( i = 0; i < PATTERN_SIZE; i++ )
  {
    if( ((p >> (PATTERN_SIZE - i - 1)) & 1) != 0 )
      c->patts[i][n] = 0xFF;
    else
      c->patts[i][n] = 0;
  }
}

/*****************************************************************************/

void Correlator_Init(corr_rec_t *c, uint64_t q) {
  int i;

  bzero( c->correlation, PATTERN_CNT * 4 );
  bzero( c->position,    PATTERN_CNT * 4 );
  bzero( c->tmp_corr,    PATTERN_CNT * 4 );

  for( i = 0; i <= 3; i++ )
    Corr_Set_Patt( c, i, Rotate_IQ_QW(q, i) );

  for( i = 0; i <= 3; i++ )
    Corr_Set_Patt( c, i + 4, Rotate_IQ_QW(Flip_IQ_QW(q), i) );
}

/*****************************************************************************/

static void Corr_Reset(corr_rec_t *c) {
  bzero( c->correlation, PATTERN_CNT * 4 );
  bzero( c->position,    PATTERN_CNT * 4 );
  bzero( c->tmp_corr,    PATTERN_CNT * 4 );
}

/*****************************************************************************/

int Corr_Correlate(corr_rec_t *c, uint8_t *data, uint32_t len) {
  int i, n, k;
  int *d;
  uint8_t *p;

  int result = -1;
  Corr_Reset( c );

  for( i = 0; (uint32_t)i < (len - PATTERN_SIZE); i++ )
  {
    for( n = 0; n < PATTERN_CNT; n++ )
      c->tmp_corr[n] = 0;
    for( k = 0; k < PATTERN_SIZE; k++ )
    {
      d = &(corr_tab[ data[i + k] ] [0]);
      p = &(c->patts[k][0]);
      //Unrolled to pattern_cnt times
      c->tmp_corr[0] += d[p[0]];
      c->tmp_corr[1] += d[p[1]];
      c->tmp_corr[2] += d[p[2]];
      c->tmp_corr[3] += d[p[3]];
      c->tmp_corr[4] += d[p[4]];
      c->tmp_corr[5] += d[p[5]];
      c->tmp_corr[6] += d[p[6]];
      c->tmp_corr[7] += d[p[7]];
    }

    for( n = 0; n < PATTERN_CNT; n++ )
      if( c->tmp_corr[n] > c->correlation[n] )
      {
        c->correlation[n] = c->tmp_corr[n];
        c->position[n] = i;
        c->tmp_corr[n] = 0;
        if( c->correlation[n] > CORR_LIMIT )
        {
          result = n;
          return( result );
        }
      }
  }

  k = 0;
  for( i = 0; i < PATTERN_CNT; i++ )
    if( c->correlation[i] > k )
    {
      result = i;
      k = c->correlation[i];
    }

  return( result );
}
