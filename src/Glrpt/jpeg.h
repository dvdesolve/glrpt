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

#ifndef JPEG_ENCODER_H
#define JPEG_ENCODER_H  1

#include "../common/common.h"


#define JPGE_MAX(a,b) (((a)>(b))?(a):(b))
#define JPGE_MIN(a,b) (((a)<(b))?(a):(b))

//typedef uint uint32_t;
typedef double dct_t;
typedef int16_t dctq_t; // quantized

// Various JPEG enums and tables.
enum
{
  M_SOF0 = 0xC0,
  M_DHT  = 0xC4,
  M_SOI  = 0xD8,
  M_EOI  = 0xD9,
  M_SOS  = 0xDA,
  M_DQT  = 0xDB,
  M_APP0 = 0xE0
};

enum
{
  DC_LUM_CODES = 12,
  AC_LUM_CODES = 256,
  DC_CHROMA_CODES = 12,
  AC_CHROMA_CODES = 256,
  MAX_HUFF_SYMBOLS  = 257,
  MAX_HUFF_CODESIZE = 32
};

static uint8_t s_zag[64] =
{
  0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,12,19,26,33,40,48,
  41,34,27,20,13,6,7,14,21,28,35,42,49,56,57,50,43,36,29,22,15,
  23,30,37,44,51,58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
};

static int16_t s_std_lum_quant[64] =
{
  16,11,12,14,12,10,16,14,13,14,18,17,16,19,24,40,26,24,22,22,24,49,35,
  37,29,40,58,51,61,60,57,51,56,55,64,72,92,78,64,68,87,69,55,56,80,
  109,81,87,95,98,103,104,103,62,77,113,121,112,100,120,92,101,103,99
};

static int16_t s_std_croma_quant[64] =
{
  17,18,18,24,21,24,47,26,26,47,99,66,56,66,99,99,99,99,99,99,99,
  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99
};

#define RGBA    TRUE  // Signal image source to be rgba type
#define RGB     FALSE // Signal image source to be rgb type

struct component
{
  uint8_t m_h_samp, m_v_samp;
  int m_last_dc_val;
};

struct huffman_table
{
  uint32_t   m_codes[256];
  uint8_t  m_code_sizes[256];
  uint8_t  m_bits[17];
  uint8_t  m_val[256];
  uint32_t m_count[256];
};

struct huffman_dcac
{
  int32_t m_quantization_table[64];
  struct huffman_table dc, ac;
};

struct image
{
  int m_x, m_y;

  float *m_pixels;
  dctq_t *m_dctqs; // quantized dcts
};

struct sym_freq
{
  uint32_t m_key;
  uint32_t m_sym_index;
};

/* JPEG encoder data structure */
struct jpeg_encoder
{
  FILE *m_pStream;
  compression_params_t m_params;
  uint8_t m_num_components;
  struct component m_comp[3];
  struct huffman_dcac m_huff[2];
  enum { JPGE_OUT_BUF_SIZE = 2048 } buff;
  uint8_t m_out_buf[JPGE_OUT_BUF_SIZE];
  uint8_t *m_pOut_buf;
  uint32_t m_out_buf_left;
  uint32_t m_bit_buffer;
  uint32_t m_bits_in;
  gboolean m_all_stream_writes_succeeded;
  int m_mcu_w, m_mcu_h;
  int m_x, m_y;
  struct image m_image[3];
};

#endif // JPEG_ENCODER

