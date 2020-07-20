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

#ifndef SDR_FILTERS_H
#define SDR_FILTERS_H

/*****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************/

/* DSP filter data */
typedef struct filter_data_t {
    /* Cutoff frequency as a fraction of sample rate */
    double cutoff;

    /* Passband ripple as a percentage */
    double ripple;

    /* Number of poles, must be even */
    uint32_t npoles;

    /* Filter type as below */
    uint32_t type;

    /* a and b coefficients of the filter */
    double *a, *b;

    /* Saved input and output values */
    double *x, *y;

    /* Ring buffer index */
    uint32_t ring_idx;

    /* Input samples buffer and its length */
    double *samples_buf;
    uint32_t samples_buf_len;
} filter_data_t;

/*****************************************************************************/

bool Init_Chebyshev_Filter(
        filter_data_t *filter_data,
        uint32_t buf_len,
        uint32_t filter_bw,
        double sample_rate,
        double ripple,
        uint32_t num_poles,
        uint32_t type);
void DSP_Filter(filter_data_t *filter_data);
void Deinit_Chebyshev_Filter(filter_data_t *data);

/*****************************************************************************/

#endif
