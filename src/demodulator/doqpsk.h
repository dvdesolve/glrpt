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

#ifndef DEMODULATOR_DOQPSK_H
#define DEMODULATOR_DOQPSK_H

/*****************************************************************************/

#include <stdint.h>

/*****************************************************************************/

void De_Interleave(uint8_t *raw, int raw_siz, uint8_t **resync, int *resync_siz);
void Make_Isqrt_Table(void);
void De_Diffcode(int8_t *buff, uint32_t length);
void Free_Isqrt_Table(void);

/*****************************************************************************/

#endif
