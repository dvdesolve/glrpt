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

#ifndef DEMOD_H
#define DEMOD_H  1

#include "../common/common.h"
#include "doqpsk.h"

/* For displaying PLL data and gauge */
#define PLL_AVE_RANGE1   0.6
#define PLL_AVE_RANGE2   3.0

/* Range factors for level gauges */
#define AGC_RANGE1      1.2
#define AGC_AVE_RANGE   2000.0

#define RESYNC_SCALE_QPSK      2000000.0
#define RESYNC_SCALE_DOQPSK    2000000.0
#define RESYNC_SCALE_IDOQPSK   2000000.0

#define DEMOD_BUF_SIZE      49152  // 3 * SOFT_FRAME_LEN
#define DEMOD_BUF_MIDL      16384  // 2 * SOFT_FRAME_LEN
#define DEMOD_BUF_LOWR      32768  // 1 * SOFT_FRAME_LEN
#define RAW_BUF_REALLOC     INTLV_BASE_LEN

#endif

