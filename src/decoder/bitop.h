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

#ifndef DECODER_BITOP_H
#define DECODER_BITOP_H

/*****************************************************************************/

#include <stdint.h>

/*****************************************************************************/

/* Bit input-output data */
typedef struct bit_io_rec_t {
    uint8_t *p;
    int pos, len;
    uint8_t cur;
    int cur_len;
} bit_io_rec_t;

/*****************************************************************************/

static inline void Bitop_AdvanceNBits(bit_io_rec_t *b, const int n) {
    b->pos += n;
}

/*****************************************************************************/

void Bitop_WriterCreate(bit_io_rec_t *w, uint8_t *bytes, int len);
void Bitop_WriteBitlistReversed(bit_io_rec_t *w, uint8_t *l, int len);
int Bitop_CountBits(uint32_t n);
uint32_t Bitop_PeekNBits(bit_io_rec_t *b, const int n);
uint32_t Bitop_FetchNBits(bit_io_rec_t *b, const int n);

/*****************************************************************************/

#endif
