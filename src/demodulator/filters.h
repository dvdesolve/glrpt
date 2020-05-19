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

#ifndef DEMODULATOR_FILTERS_H
#define DEMODULATOR_FILTERS_H

/*****************************************************************************/

#include <complex.h>
#include <stdint.h>

/*****************************************************************************/

typedef struct Filter_t {
    complex double *restrict memory;
    uint32_t fwd_count;
    uint32_t stage_no;
    double  *restrict fwd_coeff;
} Filter_t;

/*****************************************************************************/

Filter_t *Filter_RRC(uint32_t order, uint32_t factor, double osf, double alpha);
complex double Filter_Fwd(Filter_t *const self, complex double in);
void Filter_Free(Filter_t *self);

/*****************************************************************************/

#endif
