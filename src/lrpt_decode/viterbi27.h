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

#ifndef LRPT_DECODE_VITERBI27_H
#define LRPT_DECODE_VITERBI27_H

/*****************************************************************************/

#include <stddef.h>
#include <stdint.h>

/*****************************************************************************/

#define VITERBI27_POLYA     79      // 1001111
#define VITERBI27_POLYB     109     // 1101101
#define SOFT_MAX            255
#define DISTANCE_MAX        65535
#define FRAME_BITS          8192    // 1024 * 8;
#define HIGH_BIT            64
#define ENCODE_LEN          16392   // 2 * (FRAME_BITS + 8);
#define NUM_ITER            128     // HIGH_BIT << 1;
#define RENORM_INTERVAL     128     // DISTANCE_MAX / (2 * SOFT_MAX);

#define FRAME_BITS          8192
#define NUM_STATES          128
#define MIN_TRACEBACK       35      // 5*7
#define TRACEBACK_LENGTH    105     // 15*7

/*****************************************************************************/

/* Bit input-output data */
typedef struct bit_io_rec_t {
  uint8_t *p;
  int pos, len;
  uint8_t cur;
  int cur_len;
} bit_io_rec_t;

/* Viterbi decoder data */
typedef struct viterbi27_rec_t {
  int BER;

  uint16_t dist_table[4][65536];
  uint8_t  table[NUM_STATES];
  uint16_t distances[4];

  bit_io_rec_t bit_writer;

  //pair_lookup
  uint32_t pair_keys[64];      //1 shl (order-1)
  uint32_t *pair_distances;
  size_t   pair_distances_len; // My addition, size of above pointer's alloc
  uint32_t pair_outputs[16];   //1 shl (2*rate)
  uint32_t pair_outputs_len;

  uint8_t history[MIN_TRACEBACK + TRACEBACK_LENGTH][NUM_STATES];
  uint8_t fetched[MIN_TRACEBACK + TRACEBACK_LENGTH];
  int hist_index, len, renormalize_counter;

  int err_index;
  uint16_t errors[2][NUM_STATES];
  uint16_t *read_errors, *write_errors;
} viterbi27_rec_t;

/*****************************************************************************/

double Vit_Get_Percent_BER(const viterbi27_rec_t *v);
void Vit_Decode(viterbi27_rec_t *v, uint8_t *input, uint8_t *output);
void Mk_Viterbi27(viterbi27_rec_t *v);

/*****************************************************************************/

#endif
