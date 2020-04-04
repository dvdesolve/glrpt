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

#include "met_to_data.h"

#include "alib.h"
#include "correlator.h"
#include "viterbi27.h"
#include "../glrpt/utils.h"

#include <glib.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

/*------------------------------------------------------------------------*/

  void
Mtd_Init( mtd_rec_t *mtd )
{
  //sync is $1ACFFC1D,  00011010 11001111 11111100 00011101
  Correlator_Init( &(mtd->c), (uint64_t)0xfca2b63db00d9794 );
  Mk_Viterbi27( &(mtd->v) );
  mtd->pos  = 0;
  mtd->cpos = 0;
  mtd->word = 0;
  mtd->corr = 64;
}

/*------------------------------------------------------------------------*/

  static void
Do_Full_Correlate( mtd_rec_t *mtd, uint8_t *raw, uint8_t *aligned )
{
  mtd->word = (uint16_t)
    ( Corr_Correlate(&(mtd->c), &(raw[mtd->pos]), SOFT_FRAME_LEN) );
  mtd->cpos = (uint16_t)( mtd->c.position[mtd->word] );
  mtd->corr = (uint16_t)( mtd->c.correlation[mtd->word] );

  if( mtd->corr < MIN_CORRELATION )
  {
    mtd->prev_pos = mtd->pos;
    memmove( aligned, &(raw[mtd->pos]), SOFT_FRAME_LEN );
    mtd->pos += SOFT_FRAME_LEN / 4;
  }
  else
  {
    mtd->prev_pos = mtd->pos + (int)mtd->cpos;

    memmove(
        aligned,
        &(raw[mtd->pos + (int)mtd->cpos]),
        SOFT_FRAME_LEN - mtd->cpos );
    memmove(
        &(aligned[SOFT_FRAME_LEN - mtd->cpos]),
        &(raw[mtd->pos + SOFT_FRAME_LEN]),
        mtd->cpos );
    mtd->pos += SOFT_FRAME_LEN + mtd->cpos;

    Fix_Packet( aligned, SOFT_FRAME_LEN, (int)mtd->word );
  }
}

/*------------------------------------------------------------------------*/

  static void
Do_Next_Correlate( mtd_rec_t *mtd, uint8_t *raw, uint8_t *aligned )
{
  mtd->cpos = 0;
  memmove( aligned, &(raw[mtd->pos]), SOFT_FRAME_LEN );
  mtd->prev_pos = mtd->pos;
  mtd->pos += SOFT_FRAME_LEN;

  Fix_Packet( aligned, SOFT_FRAME_LEN, (int)mtd->word );
}

/*------------------------------------------------------------------------*/

static uint8_t *decoded = NULL;
  uint8_t **
ret_decoded( void )
{
  return( &decoded );
}

/*------------------------------------------------------------------------*/

  static gboolean
Try_Frame( mtd_rec_t *mtd, uint8_t *aligned )
{
  int j;
  uint8_t ecc_buf[256];
  uint32_t temp;

  if( decoded == NULL )
    mem_alloc( (void **)&decoded, HARD_FRAME_LEN );

  Vit_Decode( &(mtd->v), aligned, decoded );

  temp =
    ((uint32_t)decoded[3] << 24) +
    ((uint32_t)decoded[2] << 16) +
    ((uint32_t)decoded[1] <<  8) +
     (uint32_t)decoded[0];
  mtd->last_sync = temp;
  mtd->sig_q = (int)( round(100.0 - (Vit_Get_Percent_BER(&(mtd->v)) * 10.0)) );

  //Curiously enough, you can flip all bits in a packet
  //and get a correct ECC anyway. Check for that case
  if( Count_Bits(mtd->last_sync ^ 0xE20330E5) <
      Count_Bits(mtd->last_sync ^ 0x1DFCCF1A) )
  {
    for( j = 0; j < HARD_FRAME_LEN; j++ )
      decoded[j] ^= 0xFF;
    temp =
      ((uint32_t)decoded[3] << 24) +
      ((uint32_t)decoded[2] << 16) +
      ((uint32_t)decoded[1] <<  8) +
       (uint32_t)decoded[0];
    mtd->last_sync = temp;
  }

  for( j = 0; j < HARD_FRAME_LEN - 4; j++ )
    decoded[4 + j] ^= prand[j % 255];

  for( j = 0; j <= 3; j++ )
  {
    Ecc_Deinterleave( &(decoded[4]), ecc_buf, j, 4 );
    mtd->r[j] = Ecc_Decode( ecc_buf, 0 );
    Ecc_Interleave( ecc_buf, mtd->ecced_data, j, 4 );
  }

  return(
      (mtd->r[0] != -1) &&
      (mtd->r[1] != -1) &&
      (mtd->r[2] != -1) &&
      (mtd->r[3] != -1) );
}

/*------------------------------------------------------------------------*/

  gboolean
Mtd_One_Frame( mtd_rec_t *mtd, uint8_t *raw )
{
  uint8_t aligned[SOFT_FRAME_LEN];
  gboolean result = FALSE;

  if( mtd->cpos == 0 )
  {
    Do_Next_Correlate( mtd, raw, aligned );
    result = Try_Frame( mtd, aligned );
    if( !result ) mtd->pos -= SOFT_FRAME_LEN;
  }

  if( !result )
  {
    Do_Full_Correlate( mtd, raw, aligned );
    result = Try_Frame( mtd, aligned );
  }

  return( result );
}
