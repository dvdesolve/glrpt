/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*****************************************************************************/

#ifndef GLRPT_JPEG_H
#define GLRPT_JPEG_H

/*****************************************************************************/

#include <glib.h>

#include <stdint.h>
#include <stdio.h>

/*****************************************************************************/

/* JPEG chroma subsampling factors. Y_ONLY (grayscale images)
 * and H2V2 (color images) are the most common
 */
enum subsampling_t {
    Y_ONLY  = 0,
    H1V1    = 1,
    H2V1    = 2,
    H2V2    = 3
};

/*****************************************************************************/

/* JPEG compression parameters */
typedef struct compression_params_t {
    /* Quality: 1-100, higher is better. Typical values are around 50-95 */
    float m_quality;

    /* m_subsampling:
     * 0 = Y (grayscale) only
     * 1 = YCbCr, no subsampling (H1V1, YCbCr 1x1x1, 3 blocks per MCU)
     * 2 = YCbCr, H2V1 subsampling (YCbCr 2x1x1, 4 blocks per MCU)
     * 3 = YCbCr, H2V2 subsampling (YCbCr 4x1x1, 6 blocks per MCU - most common)
     */
    enum subsampling_t m_subsampling;

    /* Disables CbCr discrimination - only intended for testing.
     * If true, the Y quantization table is also used for the CbCr channels
     */
    gboolean m_no_chroma_discrim_flag;
} compression_params_t;

/*****************************************************************************/

gboolean jpeg_encoder_compression_parameters(
        compression_params_t *comp_params,
        float m_quality,
        enum subsampling_t m_subsampling,
        gboolean m_no_chroma_discrim_flag);
gboolean jpeg_encoder_compress_image_to_file(
        char *file_name,
        int width,
        int height,
        int num_channels,
        const uint8_t *pImage_data,
        compression_params_t *comp_params);

/*****************************************************************************/

#endif
