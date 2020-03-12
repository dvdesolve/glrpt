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

#ifndef VITERBI27_H
#define VITERBI27_H     1

#include "../common/common.h"
#include "correlator.h"

#define VITERBI27_POLYA     79      // 1001111
#define VITERBI27_POLYB     109     // 1101101
#define SOFT_MAX            255
#define DISTANCE_MAX        65535
#define FRAME_BITS          8192    // 1024 * 8;
#define HIGH_BIT            64
#define ENCODE_LEN          16392   // 2 * (FRAME_BITS + 8);
#define NUM_ITER            128     // HIGH_BIT << 1;
#define RENORM_INTERVAL     128     // DISTANCE_MAX / (2 * SOFT_MAX);

#endif

