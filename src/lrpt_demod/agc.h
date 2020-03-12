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

#ifndef AGC_H
#define AGC_H    1

#include "../common/common.h"

/* AGC default parameters */
#define AGC_WINSIZE         65536.0  // 1024*64
#define AGC_WINSIZE_1       65535.0  // 1024*64
#define AGC_TARGET          180.0
#define AGC_MAX_GAIN        20.0
#define AGC_BIAS_WINSIZE    262144.0 // 256*1024
#define AGC_BIAS_WINSIZE_1  262143.0 // 256*1024 - 1

#endif

