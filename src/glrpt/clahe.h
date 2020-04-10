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

#ifndef GLRPT_CLAHE_H
#define GLRPT_CLAHE_H

/*****************************************************************************/

#include <glib.h>

#include <stdint.h>

/*****************************************************************************/

/* Contextual regions actually used (image size is % 8) */
#define REGIONS_X   8
#define REGIONS_Y   8

/* Number of greybins for histogram ("dynamic range"). This maximum */
#define NUM_GREYBINS    256

/* Normalized cliplimit (higher values give more contrast) */
#define CLIP_LIMIT  3.0

/*****************************************************************************/

typedef unsigned char kz_pixel_t;

/*****************************************************************************/

gboolean CLAHE(
        kz_pixel_t *pImage,
        uint32_t uiXRes,
        uint32_t uiYRes,
        kz_pixel_t Min,
        kz_pixel_t Max,
        uint32_t uiNrX,
        uint32_t uiNrY,
        uint32_t uiNrBins,
        double fCliplimit);

/*****************************************************************************/

#endif
