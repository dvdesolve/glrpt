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

#ifndef DECODER_MET_JPG_H
#define DECODER_MET_JPG_H

/*****************************************************************************/

#include <stdint.h>

/*****************************************************************************/

void Mj_Dump_Image(void);
void Mj_Dec_Mcus(uint8_t *p, uint32_t apid, int pck_cnt, int mcu_id, uint8_t q);
void Mj_Init(void);

/*****************************************************************************/

#endif
