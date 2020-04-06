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

#ifndef SOAPYSDR_H
#define SOAPYSDR_H

#include <glib.h>

#include <stdint.h>

/* Range of Gain slider */
#define GAIN_SCALE  100.0

/* Range factors for level gauges */
#define AGC_GAIN_RANGE1   1.0
#define AGC_GAIN_RANGE2   0.01

#define DATA_SCALE  10.0

/* DSP Filter Parameters */
#define FILTER_RIPPLE   5.0
#define FILTER_POLES    6

gboolean SoapySDR_Init(void);
gboolean SoapySDR_Activate_Stream(void);
gboolean SoapySDR_Set_Center_Freq(uint32_t center_freq);
void SoapySDR_Set_Tuner_Gain_Mode(void);
void SoapySDR_Set_Tuner_Gain(double gain);

#endif
