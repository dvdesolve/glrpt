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

/* TODO review, may be inlines are more appropriate */
#define JPEG_MAX(a,b) (((a)>(b))?(a):(b))
#define JPEG_MIN(a,b) (((a)<(b))?(a):(b))

#define RGBA    TRUE  /* Signal image source to be rgba type */
#define RGB     FALSE /* Signal image source to be rgb type */

/*****************************************************************************/

/* JPEG chroma subsampling factors. Y_ONLY (grayscale images)
 * and H2V2 (color images) are the most common
 */
enum subsampling_t {
  Y_ONLY = 0,
  H1V1   = 1,
  H2V1   = 2,
  H2V2   = 3
};

/* JPEG compression parameters structure */
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

/* TODO review (may be move inside) */
typedef double dct_t;
typedef int16_t dctq_t; /* quantized */

/* Helper JPEG enums and tables */
enum {
  M_SOF0 = 0xC0,
  M_DHT  = 0xC4,
  M_SOI  = 0xD8,
  M_EOI  = 0xD9,
  M_SOS  = 0xDA,
  M_DQT  = 0xDB,
  M_APP0 = 0xE0
};

enum {
  DC_LUM_CODES = 12,
  AC_LUM_CODES = 256,
  DC_CHROMA_CODES = 12,
  AC_CHROMA_CODES = 256,
  MAX_HUFF_SYMBOLS  = 257,
  MAX_HUFF_CODESIZE = 32
};

struct component {
  uint8_t m_h_samp, m_v_samp;
  int m_last_dc_val;
};

struct huffman_table {
  uint32_t m_codes[256];
  uint8_t  m_code_sizes[256];
  uint8_t  m_bits[17];
  uint8_t  m_val[256];
  uint32_t m_count[256];
};

struct huffman_dcac {
  int32_t m_quantization_table[64];
  struct huffman_table dc, ac;
};

struct image {
  int m_x, m_y;

  float *m_pixels;
  dctq_t *m_dctqs; /* quantized dcts */
};

struct sym_freq {
  uint32_t m_key;
  uint32_t m_sym_index;
};

/* JPEG encoder data structure */
/* TODO may be typedef */
struct jpeg_encoder
{
  FILE *m_pStream;
  compression_params_t m_params;
  uint8_t m_num_components;
  struct component m_comp[3];
  struct huffman_dcac m_huff[2];
  enum { JPEG_OUT_BUF_SIZE = 2048 } buff;
  uint8_t m_out_buf[JPEG_OUT_BUF_SIZE];
  uint8_t *m_pOut_buf;
  uint32_t m_out_buf_left;
  uint32_t m_bit_buffer;
  uint32_t m_bits_in;
  gboolean m_all_stream_writes_succeeded;
  int m_mcu_w, m_mcu_h;
  int m_x, m_y;
  struct image m_image[3];
};

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
