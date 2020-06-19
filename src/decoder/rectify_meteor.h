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

#ifndef DECODER_RECTIFY_METEOR_H
#define DECODER_RECTIFY_METEOR_H

/*****************************************************************************/

/* Rectifying algorithms */
enum {
    R_NO = 0,
    R_W2RG,
    R_5B4AZ
};

/*****************************************************************************/

void Rectify_Images(void);

/*****************************************************************************/

#endif
