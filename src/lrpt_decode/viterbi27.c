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

#include "viterbi27.h"
#include "../common/shared.h"

/*------------------------------------------------------------------------*/

  inline double
Vit_Get_Percent_BER( const viterbi27_rec_t *v )
{
  return( 100.0 * v->BER ) / (double)FRAME_BITS;
}

/*------------------------------------------------------------------------*/

  static uint16_t
Metric_Soft_Distance(
    uint8_t hard,
    uint8_t soft_y0,
    uint8_t soft_y1 )
{
  const int mag = 255;
  int soft_x0, soft_x1;
  uint16_t result;

  switch( hard & 0x03 )
  {
    case 0:
      soft_x0 = mag;
      soft_x1 = mag;
      break;

    case 1:
      soft_x0 = -mag;
      soft_x1 =  mag;
      break;

    case 2:
      soft_x0 =  mag;
      soft_x1 = -mag;
      break;

    case 3:
      soft_x0 = -mag;
      soft_x1 = -mag;
      break;

    default:
      soft_x0 = 0;
      soft_x1 = 0; //Dewarning
  }

  //Linear distance
  result = (uint16_t)
    ( abs((int8_t)soft_y0 - soft_x0) +
      abs((int8_t)soft_y1 - soft_x1) );

  //Quadratic distance, not much better or worse.
  //result = round(sqrt(sqr(shortint(soft_y0)-soft_x0)+
  //sqr(shortint(soft_y1)-soft_x1)));

  return( result );
}

/*------------------------------------------------------------------------*/

  static void
Pair_Lookup_Create( viterbi27_rec_t *v )
{
  uint32_t inv_outputs[16];
  uint32_t output_counter, o;
  int i;

  for( i = 0; i <= 15; i++ ) inv_outputs[i] = 0;
  output_counter = 1;

  for( i = 0; i <= 63; i++ )
  {
    o = (uint32_t)( (v->table[i * 2 + 1] << 2) | v->table[i * 2] );

    if( inv_outputs[o] == 0  )
    {
      inv_outputs[o] = output_counter;
      v->pair_outputs[output_counter] = (uint32_t)o;
      output_counter++;
    }
    v->pair_keys[i] = (uint32_t)( inv_outputs[o] );
  }

  v->pair_outputs_len = output_counter;

  v->pair_distances_len = (size_t)v->pair_outputs_len;
  mem_realloc( (void **)&(v->pair_distances),
      v->pair_distances_len * sizeof(uint32_t) );
}

/*------------------------------------------------------------------------*/

  static void
Pair_Lookup_Fill_Distance( viterbi27_rec_t *v )
{
  int i;
  uint32_t c, i0, i1;

  for( i = 1; i < (int)v->pair_outputs_len; i++ )
  {
    c = v->pair_outputs[i];
    i0 = c & 3;
    i1 = c >> 2;

    v->pair_distances[i] = (uint32_t)
      ( (v->distances[i1] << 16) | v->distances[i0] );
  }
}

/*------------------------------------------------------------------------*/

  static uint32_t
History_Buffer_Search( viterbi27_rec_t *v, int search_every )
{
  uint32_t least, bestpath;
  int state;

  bestpath = 0;
  least = 0xFFFF;

  state = 0;
  while( state < NUM_STATES / 2 )
  {
    if( v->write_errors[state] < least )
    {
      least = v->write_errors[state];
      bestpath = (uint32_t)state;
    }
    state += search_every;
  }

  return( bestpath );
}

/*------------------------------------------------------------------------*/

  static void
History_Buffer_Renormalize(
    viterbi27_rec_t *v,
    uint32_t min_register )
{
  uint16_t min_distance;
  int i;

  min_distance = v->write_errors[min_register];
  for( i = 0; i < NUM_STATES / 2; i++ )
    v->write_errors[i] -= min_distance;
}

/*------------------------------------------------------------------------*/

  static void
History_Buffer_Traceback(
    viterbi27_rec_t *v,
    uint32_t bestpath,
    uint32_t min_traceback_length )
{
  int j;
  uint32_t index, fetched_index, pathbit, prefetch_index, len;
  uint8_t history;

  fetched_index = 0;
  index = (uint32_t)(v->hist_index);
  for( j = 0; j < (int)min_traceback_length; j++ )
  {
    if( index == 0 )
      index = MIN_TRACEBACK + TRACEBACK_LENGTH - 1;
    else index--;

    history = v->history[index][bestpath];
    if( history != 0 ) pathbit = HIGH_BIT;
    else pathbit = 0;
    bestpath = (bestpath | pathbit) >> 1;
  }

  prefetch_index = index;
  if( prefetch_index == 0 )
    prefetch_index = MIN_TRACEBACK + TRACEBACK_LENGTH - 1;
  else prefetch_index--;

  len = (uint32_t)(v->len);
  for( j = (int)min_traceback_length; j < (int)len; j++ )
  {
    index = prefetch_index;
    if( prefetch_index == 0 )
      prefetch_index = MIN_TRACEBACK + TRACEBACK_LENGTH - 1;
    else prefetch_index--;

    history = v->history[index][bestpath];
    if( history != 0 ) pathbit = HIGH_BIT;
    else pathbit = 0;
    bestpath = (bestpath | pathbit) >> 1;

    if( pathbit != 0 ) v->fetched[fetched_index] = 1;
    else v->fetched[fetched_index] = 0;
    fetched_index++;
  }

  Bio_Write_Bitlist_Reversed(
      &(v->bit_writer), v->fetched, (int)fetched_index );
  v->len -= fetched_index;
}

/*------------------------------------------------------------------------*/

  static void
History_Buffer_Process_Skip( viterbi27_rec_t *v, int skip )
{
  uint32_t bestpath;

  v->hist_index++;
  if( v->hist_index == MIN_TRACEBACK + TRACEBACK_LENGTH )
    v->hist_index = 0;

  v->renormalize_counter++;
  v->len++;

  if( v->renormalize_counter == RENORM_INTERVAL )
  {
    v->renormalize_counter = 0;
    bestpath = History_Buffer_Search( v, skip );
    History_Buffer_Renormalize( v, bestpath );
    if( v->len == MIN_TRACEBACK + TRACEBACK_LENGTH )
      History_Buffer_Traceback( v, bestpath, MIN_TRACEBACK );
  }
  else if( v->len == MIN_TRACEBACK + TRACEBACK_LENGTH )
  {
    bestpath = History_Buffer_Search( v, skip );
    History_Buffer_Traceback( v, bestpath, MIN_TRACEBACK );
  }
}

/*------------------------------------------------------------------------*/

  static void
Error_Buffer_Swap( viterbi27_rec_t *v )
{
  v->read_errors  = &(v->errors[v->err_index][0]);
  v->err_index    = (v->err_index + 1) % 2;
  v->write_errors = &(v->errors[v->err_index][0]);
}

/*------------------------------------------------------------------------*/

  static void
Vit_Inner( viterbi27_rec_t *v, uint8_t *soft )
{
  uint32_t highbase, low, high, base, offset, base_offset;
  int i, j;
  uint8_t *history;
  uint32_t low_key, high_key, low_concat_dist, high_concat_dist;
  uint32_t successor, low_plus_one, plus_one_successor;
  uint16_t low_past_error, high_past_error, low_error, high_error, error;
  uint16_t low_plus_one_error, high_plus_one_error, plus_one_error;
  uint8_t history_mask, plus_one_history_mask;

  for( i = 0; i <= 5; i++ )
  {
    for( j = 0; j < (1 << (i + 1)); j++ )
    {
      int idx = (soft[i * 2 + 1] << 8) + soft[i * 2];
      v->write_errors[j] =
        v->dist_table[v->table[j]][idx] + v->read_errors[j >> 1];
    }
    Error_Buffer_Swap( v );
  }

  for( i = 6; i <= FRAME_BITS - 7; i++ )
  {
    for( j = 0; j <= 3; j++ )
    {
      int idx = (soft[i * 2 + 1] << 8) + soft[i * 2];
      v->distances[j] = v->dist_table[j][idx];
    }
    history = &(v->history[v->hist_index][0]);

    Pair_Lookup_Fill_Distance( v );

    highbase = HIGH_BIT >> 1;
    low = 0;
    high = HIGH_BIT;
    base = 0;
    while( high < NUM_ITER )
    {
      offset = 0;
      base_offset = 0;
      while( base_offset < 4 )
      {
        low_key  = v->pair_keys[base + base_offset];
        high_key = v->pair_keys[highbase + base + base_offset];

        low_concat_dist  = v->pair_distances[low_key];
        high_concat_dist = v->pair_distances[high_key];

        low_past_error  = v->read_errors[base + base_offset];
        high_past_error = v->read_errors[highbase + base + base_offset];

        low_error  = (low_concat_dist  & 0xFFFF) + low_past_error;
        high_error = (high_concat_dist & 0xFFFF) + high_past_error;

        successor = low + offset;
        if( low_error <= high_error )
        {
          error = low_error;
          history_mask = 0;
        }
        else
        {
          error = high_error;
          history_mask = 1;
        }
        v->write_errors[successor] = error;
        history[successor] = history_mask;

        low_plus_one = low + offset + 1;

        low_plus_one_error  = (low_concat_dist  >> 16) + low_past_error;
        high_plus_one_error = (high_concat_dist >> 16) + high_past_error;

        plus_one_successor = low_plus_one;
        if( low_plus_one_error <= high_plus_one_error )
        {
          plus_one_error = low_plus_one_error;
          plus_one_history_mask = 0;
        }
        else
        {
          plus_one_error = high_plus_one_error;
          plus_one_history_mask = 1;
        }
        v->write_errors[plus_one_successor] = plus_one_error;
        history[plus_one_successor] = plus_one_history_mask;

        offset += 2;
        base_offset++;
      }

      low  += 8;
      high += 8;
      base += 4;
    }

    History_Buffer_Process_Skip( v, 1 );
    Error_Buffer_Swap( v );
  }
}

/*------------------------------------------------------------------------*/

  static void
Vit_Tail( viterbi27_rec_t *v, uint8_t *soft )
{
  int i, j;
  uint8_t *history;
  uint32_t skip, base_skip, highbase, low, high;
  uint32_t base, low_output, high_output;
  uint16_t low_dist, high_dist, low_past_error;
  uint16_t high_past_error, low_error, high_error;
  uint32_t successor;
  uint16_t error;
  uint8_t history_mask;


  for( i = FRAME_BITS - 6; i < FRAME_BITS; i++ )
  {
    for( j = 0; j <= 3; j++ )
    {
      int idx = (soft[i * 2 + 1] << 8) + soft[i * 2];
      v->distances[j] = v->dist_table[j][idx];
    }
    history = &(v->history[v->hist_index][0]);

    skip = 1 << ( 7 - (FRAME_BITS - i) );
    base_skip = skip >> 1;

    highbase = HIGH_BIT >> 1;
    low = 0;
    high = HIGH_BIT;
    base = 0;
    while( high < NUM_ITER )
    {
      low_output  = v->table[low];
      high_output = v->table[high];

      low_dist  = v->distances[low_output];
      high_dist = v->distances[high_output];

      low_past_error  = v->read_errors[base];
      high_past_error = v->read_errors[highbase + base];

      low_error  = low_dist  + low_past_error;
      high_error = high_dist + high_past_error;

      successor = low;
      if( low_error < high_error  )
      {
        error = low_error;
        history_mask = 0;
      }
      else
      {
        error = high_error;
        history_mask = 1;
      }
      v->write_errors[successor] = error;
      history[successor] = history_mask;

      low += skip;
      high += skip;
      base += base_skip;
    }

    History_Buffer_Process_Skip( v, (int)skip );
    Error_Buffer_Swap( v );
  }
}

/*------------------------------------------------------------------------*/

  static void
Vit_Conv_Decode(
    viterbi27_rec_t *v,
    uint8_t *msg,
    uint8_t *soft_encoded )
{
  Bit_Writer_Create( &(v->bit_writer), msg, (FRAME_BITS * 2) / 8 );

  //history_buffer
  v->len = 0;
  v->hist_index = 0;
  v->renormalize_counter = 0;

  //Error buffer
  bzero( &(v->errors[0][0]), NUM_STATES * 2 );
  bzero( &(v->errors[1][0]), NUM_STATES * 2 );
  v->err_index = 0;
  v->read_errors  = &(v->errors[0][0]);
  v->write_errors = &(v->errors[1][0]);

  Vit_Inner( v, soft_encoded );
  Vit_Tail(  v, soft_encoded );
  History_Buffer_Traceback( v, 0, 0 );
}

/*------------------------------------------------------------------------*/

  static void
Vit_Conv_Encode(
    viterbi27_rec_t *v,
    uint8_t *input,
    uint8_t *output )
{
  uint32_t sh;
  int i;
  bit_io_rec_t b;

  b.p = input;
  b.pos = 0;

  sh = 0;
  for( i = 0; i < FRAME_BITS; i++ )
  {
    sh = ( (sh << 1) | Bio_Fetch_n_Bits(&b, 1) ) & 0x7F;

    if( (v->table[sh] & 1) != 0 ) output[i * 2 + 0] = 0;
    else output[i * 2 + 0] = 255;

    if( (v->table[sh] & 2) != 0 ) output[i * 2 + 1] = 0;
    else output[i * 2 + 1] = 255;
  }
}

/*------------------------------------------------------------------------*/

  void
Vit_Decode( viterbi27_rec_t *v, uint8_t *input, uint8_t *output )
{
  int i;
  uint8_t corrected[FRAME_BITS * 2];

  Vit_Conv_Decode( v, output, input );

  //Gauge error level
  Vit_Conv_Encode( v, output, corrected );
  v->BER = 0;
  for( i = 0; i < FRAME_BITS * 2; i++ )
    v->BER += Hard_Correlate( input[i], corrected[i] ^ 0xFF );
}

/*------------------------------------------------------------------------*/

  void
Mk_Viterbi27( viterbi27_rec_t *v )
{
  int i, j;

  v->BER = 0;
  v->pair_distances = NULL; // My addition, for alloc's

  // Metric lookup table
  for( i = 0; i <= 3; i++ )
    for( j = 0; j <= 65535; j++ )
    {
      v->dist_table[i][j] =
        Metric_Soft_Distance(
            (uint8_t)i, (uint8_t)(j & 0xFF), (uint8_t)(j >> 8) );
    }

  // Polynomial table
  for( i = 0; i <= 127; i++ )
  {
    v->table[i] = 0;
    if( (Count_Bits(i & VITERBI27_POLYA) % 2) != 0  )
      v->table[i] = v->table[i] | 1;
    if( (Count_Bits(i & VITERBI27_POLYB) % 2) != 0  )
      v->table[i] = v->table[i] | 2;
  }

  Pair_Lookup_Create( v );
}

/*------------------------------------------------------------------------*/

