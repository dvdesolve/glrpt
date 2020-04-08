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

#ifndef LRPT_DEMOD_FILTERS_H
#define LRPT_DEMOD_FILTERS_H

/*****************************************************************************/

#include "demod.h"

#include <complex.h>
#include <stdint.h>

/*****************************************************************************/

Filter_t *Filter_RRC(uint32_t order, uint32_t factor, double osf, double alpha);
complex double Filter_Fwd(Filter_t *const self, complex double in);
void Filter_Free(Filter_t *self);

/*****************************************************************************/

#endif
