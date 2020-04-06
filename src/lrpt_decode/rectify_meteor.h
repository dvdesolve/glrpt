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

#ifndef LRPT_DECODE_RECTIFY_METEOR_H
#define LRPT_DECODE_RECTIFY_METEOR_H

#define	PHI_MAX   	  0.9425  // Half the max scan angle, in radians
#define	SAT_ALTITUDE  830.0   // Satellite's average altitude in km
#define EARTH_RADIUS  6370.0  // Earth's average radius in km

void Rectify_Images(void);

#endif
