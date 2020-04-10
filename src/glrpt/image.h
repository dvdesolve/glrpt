/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
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

#ifndef GLRPT_IMAGE_H
#define GLRPT_IMAGE_H

/*****************************************************************************/

#include <stdint.h>

/*****************************************************************************/

/* Normalized black value */
#define NORM_BLACK  0

/* Min and max values required in histogram normalization to leave behind
 * occasional black bands in images
 */
#define MIN_BLACK   2
#define MAX_WHITE   255

/* TODO seems like unused */
/* Minimum black value in blue APID image */
#define BLUE_MIN_BLACK  64

/*****************************************************************************/

void Normalize_Image(
        uint8_t *image_buffer,
        uint32_t image_size,
        uint8_t range_low,
        uint8_t range_high);
void Flip_Image(uint8_t *image_buffer, uint32_t image_size);
void Display_Scaled_Image(uint8_t *chan_image[], uint32_t apid, int current_y);
void Create_Combo_Image(uint8_t *combo_image);

/*****************************************************************************/

#endif
