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

#ifndef GLRPT_RC_CONFIG_H
#define GLRPT_RC_CONFIG_H

/*****************************************************************************/

#include "../common/common.h"

#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************/

/* Runtime config data storage type */
typedef struct rc_data_t {
    /* glrpt config directory, pictures directory and glade UI file */
    char glrpt_cfgs[64], glrpt_imgs[64], glrpt_glade[64];

    /* Timers: time duration (sec) for image decoding,
     * default timer duration value
     */
    uint32_t decode_timer, default_timer;

    /* Name of selected satellite */
    char satellite_name[32];

    /* SoapySDR configuration: device driver name and index */
    char device_driver[32];
    uint32_t device_index;

    /* Frequency correction factor in ppm */
    int freq_correction;

    /* SDR receiver configuration: RX frequency, RX ADC sampling rate,
     * RX ADC buffer size, low pass filter bandwidth and tuner gain
     */
    uint32_t sdr_center_freq, sdr_samplerate, sdr_buf_length, sdr_filter_bw;
    double tuner_gain;

    /* Integer FFT stride (decimation) */
    uint32_t ifft_decimate;

    /* I/Q sampling rate (sym/sec), QPSK symbol rate (sym/sec) */
    double demod_samplerate;
    uint32_t symbol_rate;

    /* Demodulator type (QPSK/OQPSK) */
    uint8_t psk_mode;

    /* Raised root cosine settings: alpha factor, filter order */
    double rrc_alpha;
    uint32_t rrc_order;

    /* Costas PLL parameters: bandwidth,
     * lower phase error threshold, upper phase error threshold
     */
    double costas_bandwidth;
    double pll_locked, pll_unlocked;

    /* Demodulator interpolation multiplier */
    uint32_t interp_factor;

    /* Channels APID */
    uint8_t apid[CHANNEL_IMAGE_NUM];

    /* Channels to combine to produce color image */
    uint8_t color_channel[CHANNEL_IMAGE_NUM];

    /* Image normalization pixel value ranges */
    uint8_t norm_range[CHANNEL_IMAGE_NUM][2];

    /* Image APIDs to invert palette (black <--> white) */
    uint32_t invert_palette[3];

    /* Scale factor to fit images in glrpt live display */
    uint32_t image_scale;

    /* Image rectification algorithm (W2RG/5B4AZ) */
    uint8_t rectify_function;

    /* Image pixel values above which we assume it is cloudy areas */
    uint8_t clouds_threshold;

    /* Max and min value of blue pixels during pseudo-colorization enhancement */
    uint8_t colorize_blue_max, colorize_blue_min;

    /* JPEG image quality */
    float jpeg_quality;
} rc_data_t;

/*****************************************************************************/

bool Load_Config(void);
bool Find_Config_Files(void);

/*****************************************************************************/

#endif
