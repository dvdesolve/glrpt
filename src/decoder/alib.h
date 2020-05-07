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

#ifndef DECODER_ALIB_H
#define DECODER_ALIB_H

/*****************************************************************************/

#include "viterbi27.h"

#include <stdint.h>

/*****************************************************************************/

static inline void Bio_Advance_n_Bits(bit_io_rec_t *b, const int n) {
    b->pos += n;
}

/*****************************************************************************/

int Count_Bits(uint32_t n);
uint32_t Bio_Peek_n_Bits(bit_io_rec_t *b, const int n);
uint32_t Bio_Fetch_n_Bits(bit_io_rec_t *b, const int n);
void Bit_Writer_Create(bit_io_rec_t *w, uint8_t *bytes, int len);
void Bio_Write_Bitlist_Reversed(bit_io_rec_t *w, uint8_t *l, int len);
int Ecc_Decode(uint8_t *idata, int pad);
void Ecc_Deinterleave(uint8_t *data, uint8_t *output, int pos, int n);
void Ecc_Interleave(uint8_t *data, uint8_t *output, int pos, int n);

/*****************************************************************************/

#endif
