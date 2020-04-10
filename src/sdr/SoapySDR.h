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

#ifndef SDR_SOAPYSDR_H
#define SDR_SOAPYSDR_H

/*****************************************************************************/

#include <glib.h>

#include <stdint.h>

/*****************************************************************************/

gboolean SoapySDR_Set_Center_Freq(uint32_t center_freq);
void SoapySDR_Set_Tuner_Gain_Mode(void);
void SoapySDR_Set_Tuner_Gain(double gain);
gboolean SoapySDR_Init(void);
gboolean SoapySDR_Activate_Stream(void);

/*****************************************************************************/

#endif
