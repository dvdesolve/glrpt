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

#ifndef DEMOD_H
#define DEMOD_H

#include "doqpsk.h"

#include <glib.h>

#include <complex.h>
#include <stdint.h>

/* For displaying PLL data and gauge */
#define PLL_AVE_RANGE1   0.6
#define PLL_AVE_RANGE2   3.0

/* Range factors for level gauges */
#define AGC_RANGE1      1.2
#define AGC_AVE_RANGE   2000.0

#define RESYNC_SCALE_QPSK      2000000.0
#define RESYNC_SCALE_DOQPSK    2000000.0
#define RESYNC_SCALE_IDOQPSK   2000000.0

/* TODO seems like mess-up; recheck and refer to SOFT_FRAME_LENGTH directly */
#define DEMOD_BUF_SIZE      49152  // 3 * SOFT_FRAME_LEN
#define DEMOD_BUF_MIDL      16384  // 1 * SOFT_FRAME_LEN
#define DEMOD_BUF_LOWR      32768  // 2 * SOFT_FRAME_LEN
#define RAW_BUF_REALLOC     INTLV_BASE_LEN

/* LRPT Demodulator data */
typedef struct Agc_t {
  double   average;
  double   gain;
  double   target_ampl;
  complex double bias;
} Agc_t;

typedef enum ModScheme {
  QPSK = 1, /* Standard QPSK as for Meteor M2 @72k sym rate */
  DOQPSK,   /* Differential Offset QPSK as for Meteor M2-2 @72k sym rate */
  IDOQPSK   /* Interleaved DOQPSK as for Meteor M2-2 @80k sym rate */
} ModScheme;

typedef struct Costas_t {
  double  nco_phase, nco_freq;
  double  alpha, beta;
  double  damping, bandwidth;
  uint8_t locked;
  double  moving_average;
  ModScheme mode;
} Costas_t;

typedef struct Filter_t {
  complex double *restrict memory;
  uint32_t fwd_count;
  uint32_t stage_no;
  double  *restrict fwd_coeff;
} Filter_t;

typedef struct Demod_t {
  Agc_t    *agc;
  Costas_t *costas;
  double    sym_period;
  uint32_t  sym_rate;
  ModScheme mode;
  Filter_t *rrc;
} Demod_t;

void Demod_Init(void);
void Demod_Deinit(void);
double Agc_Gain(double *gain);
double Signal_Level(uint32_t *level);
double Pll_Average(void);
gboolean Demodulator_Run(gpointer data);

#endif
