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

#ifndef DECODER_CORRELATOR_H
#define DECODER_CORRELATOR_H

/*****************************************************************************/

#include <stdint.h>

/*****************************************************************************/

#define PATTERN_SIZE    64
#define PATTERN_CNT     8

/*****************************************************************************/

/* Decoder correlator data */
typedef struct corr_rec_t {
    uint8_t patts[PATTERN_SIZE][PATTERN_SIZE];

    int
        correlation[PATTERN_CNT],
        tmp_corr[PATTERN_CNT],
        position[PATTERN_CNT];
} corr_rec_t;

/*****************************************************************************/

int Hard_Correlate(const uint8_t d, const uint8_t w);
void Init_Correlator_Tables(void);
void Fix_Packet(void *data, int len, int shift);
void Correlator_Init(corr_rec_t *c, uint64_t q);
int Corr_Correlate(corr_rec_t *c, uint8_t *data, uint32_t len);

/*****************************************************************************/

#endif
