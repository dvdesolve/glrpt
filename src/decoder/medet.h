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

#ifndef DECODER_MEDET_H
#define DECODER_MEDET_H

/*****************************************************************************/

#include <stdint.h>

/*****************************************************************************/

void Medet_Init(void);
void Medet_Deinit(void);
void Decode_Image(uint8_t *in_buffer, int buf_len);
double Sig_Quality(void);

/*****************************************************************************/

#endif
