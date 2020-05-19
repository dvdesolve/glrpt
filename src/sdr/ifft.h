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

#ifndef SDR_IFFT_H
#define SDR_IFFT_H

/*****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************/

bool Initialize_IFFT(int16_t width);
void Deinit_Ifft(void);
void IFFT(int16_t *data);

/*****************************************************************************/

#endif
