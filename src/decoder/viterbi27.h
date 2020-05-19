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

#ifndef DECODER_VITERBI27_H
#define DECODER_VITERBI27_H

/*****************************************************************************/

#include "bitop.h"

#include <stddef.h>
#include <stdint.h>

/*****************************************************************************/

#define FRAME_BITS          8192
#define NUM_STATES          128
#define MIN_TRACEBACK       35      // 5*7
#define TRACEBACK_LENGTH    105     // 15*7

/*****************************************************************************/

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

static inline double Vit_Get_Percent_BER(const viterbi27_rec_t *v) {
    return (100.0 * v->BER) / (double)FRAME_BITS;
}

/*****************************************************************************/

void Vit_Decode(viterbi27_rec_t *v, uint8_t *input, uint8_t *output);
void Mk_Viterbi27(viterbi27_rec_t *v);

/*****************************************************************************/

#endif
