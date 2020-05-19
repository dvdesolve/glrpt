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

#include "bitop.h"

#include <stdint.h>
#include <string.h>
#include <strings.h>

/*****************************************************************************/

static inline uint8_t NthByte(uint8_t *l, int n);

/*****************************************************************************/

static const int bitcnt[256] = {
    0, 1, 1, 2, 1, 2, 2, 3,
    1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6,
    4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6,
    4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6,
    4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6,
    4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7,
    5, 6, 6, 7, 6, 7, 7, 8
};

/*****************************************************************************/

static inline uint8_t NthByte(uint8_t *l, int n) {
    return *(l + n);
}

/*****************************************************************************/

void Bitop_WriterCreate(bit_io_rec_t *w, uint8_t *bytes, int len) {
    w->p = bytes;
    w->len = len;
    w->cur = 0;
    w->pos = 0;
    w->cur_len = 0;
}

/*****************************************************************************/

void Bitop_WriteBitlistReversed(bit_io_rec_t *w, uint8_t *l, int len) {
    int i, byte_index, close_len, full_bytes;
    uint16_t b;

    l = &l[len - 1];
    uint8_t *bytes = w->p;
    byte_index = w->pos;

    if (w->cur_len != 0) {
        close_len = 8 - w->cur_len;

        if (close_len >= len)
            close_len = len;

        b = w->cur;

        for (i = 0; i < close_len; i++) {
            b  |= l[0];
            b <<= 1;
            l  -= 1;
        }

        len -= close_len;

        if ((w->cur_len + close_len) == 8) {
            b >>= 1;
            bytes[byte_index] = (uint8_t)b;
            byte_index++;
        }
        else {
            w->cur = (uint8_t)b;
            w->cur_len += close_len;
            return;
        }
    }

    full_bytes = len / 8;

    for (i = 0; i < full_bytes; i++) {
        bytes[byte_index] = (uint8_t)
            ((NthByte(l,  0) << 7) |
             (NthByte(l, -1) << 6) |
             (NthByte(l, -2) << 5) |
             (NthByte(l, -3) << 4) |
             (NthByte(l, -4) << 3) |
             (NthByte(l, -5) << 2) |
             (NthByte(l, -6) << 1) |
             NthByte(l, -7));

        byte_index++;
        l -= 8;
    }

    len -= 8 * full_bytes;

    b = 0;

    for (i = 0; i < len; i++) {
        b |= l[0];
        b <<= 1;
        l--;
    }

    w->cur = (uint8_t)b;
    w->pos = byte_index;
    w->cur_len = len;
}

/*****************************************************************************/

int Bitop_CountBits(uint32_t n) {
    int result =
        bitcnt[n & 0xFF] +
        bitcnt[(n >> 8) & 0xFF] +
        bitcnt[(n >> 16) & 0xFF] +
        bitcnt[(n >> 24) & 0xFF];

    return result;
}

/*****************************************************************************/

uint32_t Bitop_PeekNBits(bit_io_rec_t *b, const int n) {
    int bit, i, p;
    uint32_t result = 0;

    for (i = 0; i < n; i++) {
        p = b->pos + i;
        bit = (b->p[p >> 3] >> (7 - (p & 7))) & 0x01;
        result = (result << 1) | (uint32_t)bit;
    }

    return result;
}

/*****************************************************************************/

uint32_t Bitop_FetchNBits(bit_io_rec_t *b, const int n) {
    uint32_t result = Bitop_PeekNBits(b, n);
    Bitop_AdvanceNBits(b, n);

    return result;
}
