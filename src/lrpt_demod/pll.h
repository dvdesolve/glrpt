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

#ifndef PLL_H
#define PLL_H    1

#include "../common/common.h"
#include "utils.h"

/* Costas loop default parameters */
#define FREQ_MAX            0.8     /* Maximum frequency range of locked PLL */
#define COSTAS_DAMP         0.7071  /* 1/M_SQRT2 */
#define COSTAS_INIT_FREQ    0.001
#define AVG_WINSIZE         20000.0 /* My mod, now interp. factor taken into account */
#define DELTA_WINSIZE       100.0   /* Moving Average window for pahase errors */
#define DELTA_WINSIZE_1     99.0    /* Above -1 */
#define LOCKED_WINSIZEX     10.0    /* Error Average window size multiplier (in lock) */
#define ERR_SCALE_QPSK      43.0    /* Scale factors to control magnitude of phase error */
#define ERR_SCALE_DOQPSK    80.0
#define ERR_SCALE_IDOQPSK   80.0
#define LOCKED_BW_REDUCE    4.0     /* PLL Bandwidth reduction (in lock) */
#define LOCKED_ERR_SCALE    10.0    /* Phase error scale on lock */

#endif

