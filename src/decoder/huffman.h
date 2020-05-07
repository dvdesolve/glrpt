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

#ifndef DECODER_HUFFMAN_H
#define DECODER_HUFFMAN_H

/*****************************************************************************/

#include <stdint.h>

/*****************************************************************************/

/* Decoder AC table data */
typedef struct ac_table_rec_t {
  int run, size, len;
  uint32_t mask, code;
} ac_table_rec_t;

/*****************************************************************************/

int Get_AC(const uint16_t w);
int Get_DC(const uint16_t w);
int Map_Range(const int cat, const int vl);
void Default_Huffman_Table(void);

/*****************************************************************************/

#endif
