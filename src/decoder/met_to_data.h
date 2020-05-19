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

#ifndef DECODER_MET_TO_DATA_H
#define DECODER_MET_TO_DATA_H

/*****************************************************************************/

#include "correlator.h"
#include "viterbi27.h"

#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************/

#define SOFT_FRAME_LEN  16384
#define HARD_FRAME_LEN  1024

/*****************************************************************************/

/* Decoder MTD data */
typedef struct mtd_rec_t {
    corr_rec_t c;
    viterbi27_rec_t v;

    int pos, prev_pos;
    uint8_t ecced_data[HARD_FRAME_LEN];

    uint32_t word, cpos, corr, last_sync;
    int sig_q;
    bool r[4];
} mtd_rec_t;

/*****************************************************************************/

void Mtd_Init(mtd_rec_t *mtd);
uint8_t **ret_decoded(void);
bool Mtd_One_Frame(mtd_rec_t *mtd, uint8_t *raw);

/*****************************************************************************/

#endif
