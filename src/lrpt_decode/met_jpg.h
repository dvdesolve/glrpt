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

#ifndef LRPT_DECODE_MET_JPG_H
#define LRPT_DECODE_MET_JPG_H

/*****************************************************************************/

#include <stdint.h>

/*****************************************************************************/

#define MCU_PER_PACKET  14
#define MCU_PER_LINE    196

/*****************************************************************************/

static const uint8_t standard_quantization_table[64] =
{
  16,  11,  10,  16,  24,  40,  51,  61,
  12,  12,  14,  19,  26,  58,  60,  55,
  14,  13,  16,  24,  40,  57,  69,  56,
  14,  17,  22,  29,  51,  87,  80,  62,
  18,  22,  37,  56,  68, 109, 103,  77,
  24,  35,  55,  64,  81, 104, 113,  92,
  49,  64,  78,  87, 103, 121, 120, 101,
  72,  92,  95,  98, 112, 100, 103,  99
};

static const uint8_t zigzag[64] =
{
   0,  1,  5,  6, 14, 15, 27, 28,
   2,  4,  7, 13, 16, 26, 29, 42,
   3,  8, 12, 17, 25, 30, 41, 43,
   9, 11, 18, 24, 31, 40, 44, 53,
  10, 19, 23, 32, 39, 45, 52, 54,
  20, 22, 33, 38, 46, 51, 55, 60,
  21, 34, 37, 47, 50, 56, 59, 61,
  35, 36, 48, 49, 57, 58, 62, 63
};

static const int dc_cat_off[12] = { 2, 3, 3, 3, 3, 3, 4, 5, 6, 7, 8, 9 };

enum {
  GREYSCALE_CHAN = 1,
  COLORIZED_CHAN = 3
};

/*****************************************************************************/

void Mj_Dump_Image(void);
void Mj_Dec_Mcus(uint8_t *p, uint32_t apid, int pck_cnt, int mcu_id, uint8_t q);
void Mj_Init(void);

/*****************************************************************************/

#endif
