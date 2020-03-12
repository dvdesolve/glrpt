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

#ifndef DOQPSK_H
#define DOQPSK_H   1

#include "../common/common.h"
#include "demod.h"

/* The Interleaver parameters */
/* The Interleaver branch delay INTLV_DELAY: 2048 */
#define INTLV_BRANCHES      36      // Interleaver number of branches
#define INTLV_BASE_LEN      73728   // INTLV_BRANCHES * INTLV_DELAY
#define INTLV_MESG_LEN      2654208 // INTLV_BRANCHES * INTLV_BASE_LEN
#define INTLV_DATA_LEN      72      // Number of actual interleaved symbols
#define INTLV_SYNCDATA      80      // Number of interleaved symbols + sync

#define SYNC_DET_DEPTH      8       // How many consecutive sync words to search for
#define SYNCD_BUF_MARGIN    640     // (SYNCD_DEPTH - 1) * INTLV_SYNCDATA
#define SYNCD_BLOCK_SIZ     720     // (SYNCD_DEPTH + 1) * INTLV_SYNCDATA
#define SYNCD_BUF_STEP      560     // SYNCD_DEPTH * INTLV_SYNCDATA

#define RAW_BUF_SIZE        3000000 // Raw symbols buffer size
#define DEINT_RESYNC_LEN    345792  // RAW_BUF_SIZ - INTLV_MESG_LEN

#endif

