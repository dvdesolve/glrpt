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

#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

/*****************************************************************************/

#include <limits.h>

/*****************************************************************************/

/* Control flags */
#define STATUS_RECEIVING        0x00000001 /* SDR receiver running or not     */
#define STATUS_STREAMING        0x00000002 /* SDR receiver streaming or not   */
#define STATUS_DECODING         0x00000004 /* Image decoder running or not    */
#define STATUS_DEMODULATING     0x00000008 /* LRPT demodulator running or not */
#define STATUS_SOAPYSDR_INIT    0x00000010 /* SoapySDR device init status     */
#define STATUS_PENDING          0x00000020 /* Wait for an outstanding status  */
#define STATUS_IDOQPSK_STOP     0x00000040 /* Stop decoding IDOQPSK (80k)     */
#define STATUS_FLAGS_ALL        0x0000007F /* All status flags                */
#define ALARM_ACTION_START      0x00000080 /* Start operation on SIGALRM      */
#define ALARM_ACTION_STOP       0x00000100 /* Stop operation on SIGALRM       */
#define START_STOP_TIMER        0x00000200 /* Start-Stop timer is activated   */
#define ENABLE_DECODE_TIMER     0x00000400 /* Enable decode operation timer   */
#define IMAGE_RAW               0x00000800 /* Save image in raw decoded state */
#define IMAGE_NORMALIZE         0x00001000 /* Histogram normalize wx image    */
#define IMAGE_CLAHE             0x00002000 /* CLAHE image contrast enhance    */
#define IMAGE_COLORIZE          0x00004000 /* Pseudo colorize wx image        */
#define IMAGE_COLORIZED         0x00008000 /* Pseudo colorize wx image        */
#define IMAGE_INVERT            0x00010000 /* Rotate wx image 180 degrees     */
#define IMAGES_PROCESSED        0x00020000 /* Images have been processed OK   */
#define IMAGE_RECTIFY           0x00040000 /* Rectify wx image                */
#define IMAGES_RECTIFIED        0x00080000 /* Images rectified OK             */
#define IMAGE_OUT_SPLIT         0x00100000 /* Save individual channel image   */
#define IMAGE_OUT_COMBO         0x00200000 /* Combine and save channel images */
#define FRAME_OK_ICON           0x00400000 /* Decoder frame icon showing OK   */
#define IMAGE_SAVE_JPEG         0x00800000 /* Save channel images as JPEG     */
#define IMAGE_SAVE_PPGM         0x01000000 /* Save channel image as PGM/PPM   */
#define TUNER_GAIN_AUTO         0x02000000 /* Set tuner gain to auto mode     */
#define AUTO_DETECT_SDR         0x04000000 /* Auto detect SDR device & driver */

/* Number of APID image channels */
#define CHANNEL_IMAGE_NUM   3

/* Indices for normalization range black and white values */
#define NORM_RANGE_BLACK    0
#define NORM_RANGE_WHITE    1

/* Max and min filter bandwidth */
#define MIN_BANDWIDTH   100000
#define MAX_BANDWIDTH   200000

/* Size of char arrays (strings) for text messages */
#define MESG_SIZE   128

/* Maximum time duration in sec of satellite signal processing */
#define MAX_OPERATION_TIME  1000

/* Neoklis Kyriazis' addition, width (in pixels) of image
 * METEOR_IMAGE_WIDTH = MCU_PER_LINE * 8; MCU_PER_LINE = 196
 */
#define METEOR_IMAGE_WIDTH  1568

/* TODO use booleans */
/* Return values */
#define ERROR       1
#define SUCCESS     0

/* General definitions for image processing */
#define MAX_FILE_NAME   PATH_MAX /* Max length for filenames */

/* Safe fallback */
#ifndef M_2PI
#define M_2PI 6.28318530717958647692
#endif

/*****************************************************************************/

/* Filter types */
enum {
    FILTER_LOWPASS = 0,
    FILTER_HIGHPASS,
    FILTER_BANDPASS
};

/* Image channels (0-2) */
enum {
    RED = 0,
    GREEN,
    BLUE
};

/* Flags to select images for output */
enum {
    OUT_COMBO = 1,
    OUT_SPLIT,
    OUT_BOTH
};

/* Flags to indicate image file type to save as */
enum {
    SAVEAS_JPEG = 1,
    SAVEAS_PGM,
    SAVEAS_BOTH
};

/* Flags to indicate images to be saved */
enum {
    CH0 = 0,
    CH1,
    CH2,
    COMBO
};

/*****************************************************************************/

#endif
