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

#ifndef LRPT_DECODE_CORRELATOR_H
#define LRPT_DECODE_CORRELATOR_H

/*****************************************************************************/

#include "met_to_data.h"

#include <stdint.h>

/*****************************************************************************/

#define CORR_LIMIT          55

/*****************************************************************************/

int Hard_Correlate(const uint8_t d, const uint8_t w);
void Init_Correlator_Tables(void);
void Fix_Packet(void *data, int len, int shift);
void Correlator_Init(corr_rec_t *c, uint64_t q);
int Corr_Correlate(corr_rec_t *c, uint8_t *data, uint32_t len);

/*****************************************************************************/

#endif
