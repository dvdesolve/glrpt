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

#ifndef DECODER_ECC_H
#define DECODER_ECC_H

/*****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************/

bool Ecc_Decode(uint8_t *idata, int pad);
void Ecc_Deinterleave(uint8_t *data, uint8_t *output, int pos, int n);
void Ecc_Interleave(uint8_t *data, uint8_t *output, int pos, int n);

/*****************************************************************************/

#endif
