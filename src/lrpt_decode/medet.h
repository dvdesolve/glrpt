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

#ifndef LRPT_DECODE_MEDET_H
#define LRPT_DECODE_MEDET_H

#include <stdint.h>

#define SIG_QUAL_RANGE  100.0

double Sig_Quality(void);
void Medet_Init(void);
void Medet_Deinit(void);
void Decode_Image(uint8_t *in_buffer, int buf_len);

#endif
