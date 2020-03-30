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
#define SOAPYSDR_H      1

#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include "../common/common.h"
#include "../glrpt/display.h"
#include "ifft.h"

/* Range of Gain slider */
#define GAIN_SCALE  100.0

/* Range factors for level gauges */
#define AGC_GAIN_RANGE1   1.0
#define AGC_GAIN_RANGE2   0.01
#define AGC_AVE_RANGE     5000.0

#define DATA_SCALE  10.0

#endif

