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

#include <glib.h>

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************/

#define CFG_STRLEN_MAX  80

/*****************************************************************************/

/* Runtime config file entry */
typedef struct rc_cfg_t {
    char *name;
    char *path;
} rc_cfg_t;

/* Runtime config data storage type */
typedef struct rc_data_t {
    /* Satellite name and optional comment in config file */
    char sat_name[CFG_STRLEN_MAX + 1], comment[CFG_STRLEN_MAX + 1];

    /* SoapySDR configuration: device driver and index */
    char device_driver[CFG_STRLEN_MAX + 1];
    uint8_t device_index;

    /* SDR receiver configuration:
     * RX frequency, low-pass filter bandwidth,
     * gain, frequency correction factor (ppm)
     */
    uint32_t sdr_center_freq, sdr_filter_bw;
    double tuner_gain, freq_correction;

    /* Raised root cosine settings: filter order and alpha factor */
    uint32_t rrc_order;
    double rrc_alpha;

    /* Demodulator interpolation multiplier */
    uint32_t interp_factor;

    /* Demodulator type (QPSK/DOQPSK/IDOQPSK) and symbol rate (Sym/s) */
    uint8_t psk_mode;
    uint32_t symbol_rate;

    /* Costas PLL parameters: bandwidth,
     * lower phase error threshold, upper phase error threshold
     */
    double costas_bandwidth;
    double pll_locked, pll_unlocked;

    /* Channels APIDs and APIDs for palette inversion */
    uint8_t apid[CHANNEL_IMAGE_NUM];
    uint32_t invert_palette[3]; /* TODO why 3 and uint32_t? */

    /* Channels to combine to produce color image */
    uint8_t color_channel[CHANNEL_IMAGE_NUM]; /* TODO should use exactly 3 */

    /* Timers: time duration (sec) for image decoding,
     * default timer duration value
     */
    /* TODO why do we need uint32_t? */
    uint32_t decode_timer, default_timer;

    /* Image normalization pixel value ranges */
    uint8_t norm_range[CHANNEL_IMAGE_NUM][2]; /* TODO should be exactly 3 */

    /* Max and min value of blue pixels during pseudo-colorization enhancement */
    uint8_t colorize_blue_max, colorize_blue_min;

    /* Image pixel values above which we assume it is cloudy areas */
    uint8_t clouds_threshold;

    /* Image rectification algorithm (W2RG/5B4AZ) */
    uint8_t rectify_function;

    /* JPEG image quality */
    int jpeg_quality;

    /* Scale factor to fit images in glrpt live display */
    /* TODO do we need uint32_t? */
    uint32_t image_scale;
} rc_data_t;

/*****************************************************************************/

/* Accessible configs */
extern rc_cfg_t *glrpt_cfg_list;

/* Program-wide directories */
extern char
    glrpt_cfg_dir[PATH_MAX + 1],
    glrpt_ucfg_dir[PATH_MAX + 1],
    glrpt_img_dir[PATH_MAX + 1];

/*****************************************************************************/

gboolean loadConfig(gpointer f_path);
bool findConfigFiles(void);

/*****************************************************************************/

#endif
