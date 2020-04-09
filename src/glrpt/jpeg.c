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

#include "jpeg.h"

#include "utils.h"

#include <glib.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************/

static void image_init(struct image *img);
static void image_deinit(struct image *img);
static inline void image_set_px(struct image *img, float px, int x, int y);
static inline float image_get_px(struct image *img, int x, int y);
static dctq_t *image_get_dctq(struct image *img, int x, int y);
static void image_load_block(struct image *img, dct_t *pDst, int x, int y);
static inline dct_t image_blend_dual(
        struct image *img,
        int x,
        int y,
        struct image *luma);
static inline dct_t image_blend_quad(
        struct image *img,
        int x,
        int y,
        struct image *luma);
static void image_subsample(struct image *img, struct image *luma, int v_samp);
static void RGB_to_YCC(
        struct image *img,
        const uint8_t *pSrc,
        gboolean ch_a, // If source is rgba
        int width,
        int y);
static void RGB_to_Y(
        struct image *img,
        const uint8_t *pSrc,
        gboolean ch_a, // If source is rgba
        int width,
        int y);
static void Y_to_YCC(
        struct image *img,
        const uint8_t *pSrc,
        int width,
        int y);
static void dct(dct_t *data);
static inline struct sym_freq *radix_sort_syms(
        uint32_t num_syms,
        struct sym_freq *pSyms0,
        struct sym_freq *pSyms1);
static void calculate_minimum_redundancy(struct sym_freq *A, int n);
static void huffman_enforce_max_code_size(
        int *pNum_codes,
        int code_list_len,
        int max_code_size);
static void huffman_table_optimize(struct huffman_table *table, int table_len);
static void huffman_table_compute(struct huffman_table *table);
static gboolean jpeg_encoder_put_buf(const void *pBuf, int len, FILE *stream);
static void jpeg_encoder_emit_byte(struct jpeg_encoder *enc, uint8_t c);
static void jpeg_encoder_emit_word(struct jpeg_encoder *enc, uint32_t i);
static void jpeg_encoder_emit_marker(struct jpeg_encoder *enc, int marker);
static void jpeg_encoder_emit_jfif_app0(struct jpeg_encoder *enc);
static void jpeg_encoder_emit_dqt(struct jpeg_encoder *enc);
static void jpeg_encoder_emit_sof(struct jpeg_encoder *enc);
static void jpeg_encoder_emit_dht(
        struct jpeg_encoder *enc,
        uint8_t *bits,
        uint8_t *val,
        int index,
        gboolean ac_flag);
static void jpeg_encoder_emit_dhts(struct jpeg_encoder *enc);
static void jpeg_encoder_emit_sos(struct jpeg_encoder *enc);
static void jpeg_encoder_emit_start_markers(struct jpeg_encoder *enc);
static void jpeg_encoder_compute_quant_table(
        struct jpeg_encoder *enc,
        int32_t *pDst,
        int16_t *pSrc);
static void jpeg_encoder_reset_last_dc(struct jpeg_encoder *enc);
static void jpeg_encoder_compute_huffman_tables(struct jpeg_encoder *enc);
static gboolean jpeg_encoder_jpg_open(
        struct jpeg_encoder *enc,
        int p_x_res,
        int p_y_res);
static inline dctq_t round_to_zero(const dct_t j, const int32_t quant);
static void jpeg_encoder_quantize_pixels(
        struct jpeg_encoder *enc,
        dct_t *pSrc,
        dctq_t *pDst,
        const int32_t *quant);
static void jpeg_encoder_flush_output_buffer(struct jpeg_encoder *enc);
static inline uint32_t bit_count(int temp1);
static void jpeg_encoder_put_bits(
        struct jpeg_encoder *enc,
        uint32_t bits,
        uint32_t len);
static void jpeg_encoder_put_signed_int_bits(
        struct jpeg_encoder *enc,
        int num,
        uint32_t len);
static void jpeg_encoder_code_block(
        struct jpeg_encoder *enc,
        dctq_t *src,
        struct huffman_dcac *huff,
        struct component *comp,
        gboolean write);
static void jpeg_encoder_code_mcu_row(
        struct jpeg_encoder *enc,
        int y,
        gboolean write);
static gboolean jpeg_encoder_emit_end_markers(struct jpeg_encoder *enc);
static gboolean jpeg_encoder_compress_image(struct jpeg_encoder *enc);
static void jpeg_encoder_load_mcu_Y(
        struct jpeg_encoder *enc,
        const uint8_t *pSrc,
        int width,
        int bpp,
        int y);
static void jpeg_encoder_load_mcu_YCC(
        struct jpeg_encoder *enc,
        const uint8_t *pSrc,
        int width,
        int bpp,
        int y);
static void jpeg_encoder_clear(struct jpeg_encoder *enc);
static void jpeg_encoder_deinit(struct jpeg_encoder *enc);
static inline gboolean params_check(compression_params_t *parm);
static gboolean jpeg_encoder_init(
        struct jpeg_encoder *enc,
        FILE *pStream,
        int width,
        int height,
        compression_params_t *comp_params);
static gboolean jpeg_encoder_read_image(
        struct jpeg_encoder *enc,
        const uint8_t *image_data,
        int width,
        int height,
        int bpp);

/*****************************************************************************/

static int16_t s_std_croma_quant[64] = {
  17,18,18,24,21,24,47,26,26,47,99,66,56,66,99,99,99,99,99,99,99,
  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
  99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99
};

static int16_t s_std_lum_quant[64] = {
  16,11,12,14,12,10,16,14,13,14,18,17,16,19,24,40,26,24,22,22,24,49,35,
  37,29,40,58,51,61,60,57,51,56,55,64,72,92,78,64,68,87,69,55,56,80,
  109,81,87,95,98,103,104,103,62,77,113,121,112,100,120,92,101,103,99
};

static uint8_t s_zag[64] = {
  0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,12,19,26,33,40,48,
  41,34,27,20,13,6,7,14,21,28,35,42,49,56,57,50,43,36,29,22,15,
  23,30,37,44,51,58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
};

/*****************************************************************************/

static void image_init(struct image *img) {
  mem_alloc( (void **)&(img->m_pixels),
      sizeof(float) * (size_t)(img->m_x * img->m_y) );
  mem_alloc( (void **)&(img->m_dctqs),
      sizeof(dctq_t) * (size_t)(img->m_x * img->m_y) );
}

/*****************************************************************************/

static void image_deinit(struct image *img) {
  free_ptr( (void **)&(img->m_pixels) );
  free_ptr( (void **)&(img->m_dctqs) );
}

/*****************************************************************************/

static inline void image_set_px(struct image *img, float px, int x, int y) {
  img->m_pixels[y * img->m_x + x] = px;
}

/*****************************************************************************/

static inline float image_get_px(struct image *img, int x, int y) {
  float px = img->m_pixels[y * img->m_x + x];
  return( px );
}

/*****************************************************************************/

static dctq_t *image_get_dctq(struct image *img, int x, int y) {
  dctq_t *dctq = &(img->m_dctqs[64 * (y/8 * img->m_x/8 + x/8)]);
  return( dctq );
}

/*****************************************************************************/

static void image_load_block(struct image *img, dct_t *pDst, int x, int y) {
  for( int i = 0; i < 8; i++ )
  {
    pDst[0] = (double)image_get_px( img, x + 0, y + i );
    pDst[1] = (double)image_get_px( img, x + 1, y + i );
    pDst[2] = (double)image_get_px( img, x + 2, y + i );
    pDst[3] = (double)image_get_px( img, x + 3, y + i );
    pDst[4] = (double)image_get_px( img, x + 4, y + i );
    pDst[5] = (double)image_get_px( img, x + 5, y + i );
    pDst[6] = (double)image_get_px( img, x + 6, y + i );
    pDst[7] = (double)image_get_px( img, x + 7, y + i );
    pDst += 8;
  }
}

/*****************************************************************************/

static inline dct_t image_blend_dual(
        struct image *img,
        int x,
        int y,
        struct image *luma) {
  float pxa, pxb;
  dct_t blend;

  pxa = image_get_px( luma, x, y );
  dct_t a = 129 - abs( (int)pxa );
  pxb = image_get_px( luma, x + 1, y );
  dct_t b = 129 - abs( (int)pxb );

  pxa = image_get_px( img, x, y );
  pxb = image_get_px( img, x + 1, y );

  blend = ( (dct_t)pxa * a + (dct_t)pxb * b ) / ( a + b );

  return( blend );
}

/*****************************************************************************/

static inline dct_t image_blend_quad(
        struct image *img,
        int x,
        int y,
        struct image *luma) {
  float pxa, pxb, pxc, pxd;
  dct_t blend;

  pxa = image_get_px( luma, x, y );
  dct_t a = 129 - abs( (int)pxa );
  pxb = image_get_px( luma, x + 1, y );
  dct_t b = 129 - abs( (int)pxb );
  pxc = image_get_px( luma, x, y + 1 );
  dct_t c = 129 - abs( (int)pxc );
  pxd = image_get_px( luma, x + 1, y + 1 );
  dct_t d = 129 - abs( (int)pxd );

  pxa = image_get_px( img, x, y );
  pxb = image_get_px( img, x + 1, y );
  pxc = image_get_px( img, x, y + 1 );
  pxd = image_get_px( img, x + 1, y + 1 );

  blend =
    (dct_t)pxa * a + (dct_t)pxb * b +
    (dct_t)pxc * c + (dct_t)pxd * d;
  blend /= a + b + c + d;

  return( blend );
}

/*****************************************************************************/

static void image_subsample(struct image *img, struct image *luma, int v_samp) {
  if( v_samp == 2 )
  {
    for( int y = 0; y < img->m_y; y += 2 )
    {
      for( int x = 0; x < img->m_x; x += 2 )
      {
        img->m_pixels[img->m_x / 4 * y + x / 2] =
          (float)image_blend_quad( img, x, y, luma );
      }
    }
    img->m_x /= 2;
    img->m_y /= 2;
  }
  else
  {
    for( int y = 0; y < img->m_y; y++ )
    {
      for( int x = 0; x < img->m_x; x += 2 )
      {
        img->m_pixels[img->m_x / 2 * y + x / 2] =
          (float)image_blend_dual( img, x, y, luma );
      }
    }
    img->m_x /= 2;
  }
}

/*****************************************************************************/

static void RGB_to_YCC(
        struct image *img,
        const uint8_t *pSrc,
        gboolean ch_a, // If source is rgba
        int width,
        int y) {
  int r, g, b;
  float px;
  int idx = 0;

  for( int x = 0; x < width; x++ )
  {
    r = pSrc[idx++];
    g = pSrc[idx++];
    b = pSrc[idx++];
    if( ch_a ) idx++; // Dump channel a

    px = (float)( (0.299 * r) + (0.587 * g) + (0.114 * b) - 128.0 );
    image_set_px( &img[0], px, x, y );
    px = (float)( -(0.168736 * r) - (0.331264 * g) + (0.5 * b) );
    image_set_px( &img[1], px, x, y );
    px = (float)( (0.5 * r) - (0.418688 * g) - (0.081312 * b) );
    image_set_px( &img[2], px, x, y );
  }
}

/*****************************************************************************/

static void RGB_to_Y(
        struct image *img,
        const uint8_t *pSrc,
        gboolean ch_a, // If source is rgba
        int width,
        int y) {
  float r, g, b, px;
  int idx = 0;

  for( int x = 0; x < width; x++ )
  {
    r = (float)pSrc[idx++];
    g = (float)pSrc[idx++];
    b = (float)pSrc[idx++];
    if( ch_a ) idx++; // Dump channel a

    px = (r * 0.299f) + (g * 0.587f) + (b * 0.114f) - 128.0f;
    image_set_px( img, px, x, y );
  }
}

/*****************************************************************************/

static void Y_to_YCC(
        struct image *img,
        const uint8_t *pSrc,
        int width,
        int y) {
  for( int x = 0; x < width; x++ )
  {
    image_set_px( &img[0], pSrc[x] - 128.0f, x, y );
    image_set_px( &img[1], 0, x, y );
    image_set_px( &img[2], 0, x, y );
  }
}

/*****************************************************************************/

// Forward DCT
static void dct(dct_t *data) {
  dct_t
    z1, z2, z3, z4, z5, tmp0, tmp1, tmp2, tmp3, tmp4, tmp5,
    tmp6, tmp7, tmp10, tmp11, tmp12, tmp13, *data_ptr;

  data_ptr = data;

  for( int c = 0; c < 8; c++ )
  {
    tmp0 = data_ptr[0] + data_ptr[7];
    tmp7 = data_ptr[0] - data_ptr[7];
    tmp1 = data_ptr[1] + data_ptr[6];
    tmp6 = data_ptr[1] - data_ptr[6];
    tmp2 = data_ptr[2] + data_ptr[5];
    tmp5 = data_ptr[2] - data_ptr[5];
    tmp3 = data_ptr[3] + data_ptr[4];
    tmp4 = data_ptr[3] - data_ptr[4];
    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    data_ptr[0] = tmp10 + tmp11;
    data_ptr[4] = tmp10 - tmp11;
    z1 = (tmp12 + tmp13) * 0.541196100;
    data_ptr[2] = z1 + tmp13 *  0.765366865;
    data_ptr[6] = z1 + tmp12 * -1.847759065;
    z1 = tmp4 + tmp7;
    z2 = tmp5 + tmp6;
    z3 = tmp4 + tmp6;
    z4 = tmp5 + tmp7;
    z5 = (z3 + z4) * 1.175875602;
    tmp4 *= 0.298631336;
    tmp5 *= 2.053119869;
    tmp6 *= 3.072711026;
    tmp7 *= 1.501321110;
    z1 *= -0.899976223;
    z2 *= -2.562915447;
    z3 *= -1.961570560;
    z4 *= -0.390180644;
    z3 += z5;
    z4 += z5;
    data_ptr[7] = tmp4 + z1 + z3;
    data_ptr[5] = tmp5 + z2 + z4;
    data_ptr[3] = tmp6 + z2 + z3;
    data_ptr[1] = tmp7 + z1 + z4;
    data_ptr += 8;
  }

  data_ptr = data;

  for( int c = 0; c < 8; c++ )
  {
    tmp0 = data_ptr[8 * 0] + data_ptr[8 * 7];
    tmp7 = data_ptr[8 * 0] - data_ptr[8 * 7];
    tmp1 = data_ptr[8 * 1] + data_ptr[8 * 6];
    tmp6 = data_ptr[8 * 1] - data_ptr[8 * 6];
    tmp2 = data_ptr[8 * 2] + data_ptr[8 * 5];
    tmp5 = data_ptr[8 * 2] - data_ptr[8 * 5];
    tmp3 = data_ptr[8 * 3] + data_ptr[8 * 4];
    tmp4 = data_ptr[8 * 3] - data_ptr[8 * 4];
    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    data_ptr[8 * 0] = ( tmp10 + tmp11 ) / 8.0;
    data_ptr[8 * 4] = ( tmp10 - tmp11 ) / 8.0;
    z1 = ( tmp12 + tmp13 ) * 0.541196100;
    data_ptr[8 * 2] = ( z1 + tmp13 *  0.765366865 ) / 8.0;
    data_ptr[8 * 6] = ( z1 + tmp12 * -1.847759065 ) / 8.0;
    z1 = tmp4 + tmp7;
    z2 = tmp5 + tmp6;
    z3 = tmp4 + tmp6;
    z4 = tmp5 + tmp7;
    z5 = (z3 + z4) * 1.175875602;
    tmp4 *= 0.298631336;
    tmp5 *= 2.053119869;
    tmp6 *= 3.072711026;
    tmp7 *= 1.501321110;
    z1 *= -0.899976223;
    z2 *= -2.562915447;
    z3 *= -1.961570560;
    z4 *= -0.390180644;
    z3 += z5;
    z4 += z5;
    data_ptr[8 * 7] = ( tmp4 + z1 + z3 ) / 8.0;
    data_ptr[8 * 5] = ( tmp5 + z2 + z4 ) / 8.0;
    data_ptr[8 * 3] = ( tmp6 + z2 + z3 ) / 8.0;
    data_ptr[8 * 1] = ( tmp7 + z1 + z4 ) / 8.0;
    data_ptr++;
  }
}

/*****************************************************************************/

// Radix sorts sym_freq[] array by 32-bit key m_key.
// Returns pointer to sorted values.
static inline struct sym_freq *radix_sort_syms(
        uint32_t num_syms,
        struct sym_freq *pSyms0,
        struct sym_freq *pSyms1) {
  const uint32_t cMaxPasses = 4;
  uint32_t hist[256 * 4];

  memset( (void *)hist, 0, sizeof(hist) );
  for( uint32_t i = 0; i < num_syms; i++ )
  {
    uint32_t freq = pSyms0[i].m_key;
    hist[freq & 0xFF]++;
    hist[256     + ((freq >> 8)  & 0xFF)]++;
    hist[256 * 2 + ((freq >> 16) & 0xFF)]++;
    hist[256 * 3 + ((freq >> 24) & 0xFF)]++;
  }

  struct sym_freq *pCur_syms = pSyms0, *pNew_syms = pSyms1;
  uint32_t total_passes = cMaxPasses;

  while(
      (total_passes > 1) &&
      (num_syms == hist[(total_passes - 1) * 256]) )
  {
    total_passes--;
  }
  for( uint32_t pass_shift = 0, pass = 0;
      pass < total_passes;
      pass++, pass_shift += 8 )
  {
    const uint32_t *pHist = &hist[pass << 8];
    uint32_t offsets[256], cur_ofs = 0;
    for( uint32_t i = 0; i < 256; i++ )
    {
      offsets[i] = cur_ofs;
      cur_ofs += pHist[i];
    }
    for( uint32_t i = 0; i < num_syms; i++ )
    {
      pNew_syms[offsets[
        (pCur_syms[i].m_key >> pass_shift) & 0xFF]++] = pCur_syms[i];
    }
    struct sym_freq *t = pCur_syms;
    pCur_syms = pNew_syms;
    pNew_syms = t;
  }
  return( pCur_syms );
}

/*****************************************************************************/

// calculate_minimum_redundancy() originally written by:
// Alistair Moffat, alistair@cs.mu.oz.au, Jyrki Katajainen,
// jyrki@diku.dk, November 1996.
static void calculate_minimum_redundancy(struct sym_freq *A, int n) {
  int root, leaf, next, avbl, used, dpth;

  if( n == 0 )
  {
    return;
  }
  else if( n == 1 )
  {
    A[0].m_key = 1;
    return;
  }

  A[0].m_key += A[1].m_key;
  root = 0;
  leaf = 2;

  for( next = 1; next < n - 1; next++ )
  {
    if( (leaf >= n) || (A[root].m_key < A[leaf].m_key) )
    {
      A[next].m_key   = A[root].m_key;
      A[root++].m_key = (uint32_t)next;
    }
    else
    {
      A[next].m_key = A[leaf++].m_key;
    }

    if( (leaf >= n) || ((root < next) && (A[root].m_key < A[leaf].m_key)) )
    {
      A[next].m_key += A[root].m_key;
      A[root++].m_key = (uint32_t)next;
    }
    else
    {
      A[next].m_key += A[leaf++].m_key;
    }
  }

  A[n - 2].m_key = 0;
  for( next = n - 3; next >= 0; next-- )
  {
    A[next].m_key = A[A[next].m_key].m_key + 1;
  }

  avbl = 1;
  used = dpth = 0;
  root = n - 2;
  next = n - 1;

  while( avbl > 0 )
  {
    while( (root >= 0) && ((int)A[root].m_key == dpth) )
    {
      used++;
      root--;
    }

    while( avbl > used )
    {
      A[next--].m_key = (uint32_t)dpth;
      avbl--;
    }

    avbl = 2 * used;
    dpth++;
    used = 0;
  }
}

/*****************************************************************************/

// Limits canonical Huffman code table's
// max code size to max_code_size.
static void huffman_enforce_max_code_size(
        int *pNum_codes,
        int code_list_len,
        int max_code_size) {
  if( code_list_len <= 1 )
  {
    return;
  }

  for( int i = max_code_size + 1; i <= MAX_HUFF_CODESIZE; i++ )
  {
    pNum_codes[max_code_size] += pNum_codes[i];
  }

  uint32_t total = 0;
  for( int i = max_code_size; i > 0; i-- )
  {
    total += ( ((uint32_t)pNum_codes[i]) << (max_code_size - i) );
  }

  while( total != (1UL << max_code_size) )
  {
    pNum_codes[max_code_size]--;
    for( int i = max_code_size - 1; i > 0; i-- )
    {
      if( pNum_codes[i])
      {
        pNum_codes[i]--;
        pNum_codes[i + 1] += 2;
        break;
      }
    }
    total--;
  }
}

/*****************************************************************************/

// Generates an optimized huffman table.
static void huffman_table_optimize(struct huffman_table *table, int table_len) {
  struct sym_freq
    syms0[MAX_HUFF_SYMBOLS],
    syms1[MAX_HUFF_SYMBOLS];

    // dummy symbol, assures that no valid code contains all 1's
    syms0[0].m_key = 1;
    syms0[0].m_sym_index = 0;

    int num_used_syms = 1;
    for( int i = 0; i < table_len; i++ )
    {
      if( table->m_count[i] )
      {
        syms0[num_used_syms].m_key = table->m_count[i];
        syms0[num_used_syms].m_sym_index = (uint32_t)(i + 1);
        num_used_syms++;
      }
    }
    struct sym_freq *pSyms =
      radix_sort_syms( (uint32_t)num_used_syms, syms0, syms1 );
    calculate_minimum_redundancy( pSyms, num_used_syms );

    // Count the # of symbols of each code size.
    int num_codes[1 + MAX_HUFF_CODESIZE];
    memset( (void *)num_codes, 0, sizeof(num_codes) );
    for( int i = 0; i < num_used_syms; i++ )
    {
      num_codes[pSyms[i].m_key]++;
    }

    // the maximum possible size of a JPEG Huffman code
    // (valid range is [9,16] - 9 vs. 8 because of the dummy symbol)
    const uint32_t JPEG_CODE_SIZE_LIMIT = 16;
    huffman_enforce_max_code_size(
        num_codes, num_used_syms, JPEG_CODE_SIZE_LIMIT );

    // Compute m_huff_bits array, which contains the # of symbols per code size.
    memset( (void *)(table->m_bits), 0, sizeof(table->m_bits) );
    for( int i = 1; i <= (int)JPEG_CODE_SIZE_LIMIT; i++ )
    {
      table->m_bits[i] = (uint8_t)( num_codes[i] );
    }

    // Remove the dummy symbol added above, which must be in largest bucket.
    for( int i = JPEG_CODE_SIZE_LIMIT; i >= 1; i-- )
    {
      if( table->m_bits[i] )
      {
        table->m_bits[i]--;
        break;
      }
    }

    // Compute the m_huff_val array, which contains the
    // symbol indices sorted by code size (smallest to largest).
    for( int i = num_used_syms - 1; i >= 1; i-- )
    {
      table->m_val[num_used_syms - 1 - i] =
        (uint8_t)( pSyms[i].m_sym_index - 1 );
    }
}

/*****************************************************************************/

// Compute the actual canonical Huffman codes/code
// sizes given the JPEG huff bits and val arrays.
static void huffman_table_compute(struct huffman_table *table) {
  int last_p, si;
  uint8_t huff_size[257];
  uint32_t huff_code[257];
  uint32_t code;

  int p = 0;
  for( uint8_t l = 1; l <= 16; l++ )
  {
    for( int i = 1; i <= table->m_bits[l]; i++ )
    {
      huff_size[p++] = l;
    }
  }

  huff_size[p] = 0;
  last_p = p; // write sentinel

  code = 0;
  si = huff_size[0];
  p = 0;

  while( huff_size[p] )
  {
    while( huff_size[p] == si )
    {
      huff_code[p++] = code++;
    }
    code <<= 1;
    si++;
  }

  memset( (void *)table->m_codes, 0,
      sizeof(table->m_codes[0]) * 256 );
  memset( (void *)table->m_code_sizes, 0,
      sizeof(table->m_code_sizes[0]) * 256 );
  for( p = 0; p < last_p; p++ )
  {
    table->m_codes[table->m_val[p]] = huff_code[p];
    table->m_code_sizes[table->m_val[p]] = huff_size[p];
  }
}

/*****************************************************************************/

static gboolean jpeg_encoder_put_buf(const void *pBuf, int len, FILE *stream) {
  gboolean status = ( fwrite(pBuf, (size_t)len, 1, stream) == 1 );
  return( status );
}

/*****************************************************************************/

// JPEG marker generation.
static void jpeg_encoder_emit_byte(struct jpeg_encoder *enc, uint8_t c) {
  gboolean ret = jpeg_encoder_put_buf(
      (void *)&c, sizeof(c), enc->m_pStream );

  enc->m_all_stream_writes_succeeded =
    enc->m_all_stream_writes_succeeded && ret;
}

/*****************************************************************************/

static void jpeg_encoder_emit_word(struct jpeg_encoder *enc, uint32_t i) {
  jpeg_encoder_emit_byte( enc, (uint8_t)(i >> 8) );
  jpeg_encoder_emit_byte( enc, (uint8_t)(i & 0xFF) );
}

/*****************************************************************************/

static void jpeg_encoder_emit_marker(struct jpeg_encoder *enc, int marker) {
  jpeg_encoder_emit_byte( enc, (uint8_t)(0xFF) );
  jpeg_encoder_emit_byte( enc, (uint8_t)(marker) );
}

/*****************************************************************************/

// Emit JFIF marker
static void jpeg_encoder_emit_jfif_app0(struct jpeg_encoder *enc) {
  jpeg_encoder_emit_marker(enc, M_APP0);
  jpeg_encoder_emit_word( enc, 2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1 );

  /* Identifier: ASCII "JFIF" */
  jpeg_encoder_emit_byte( enc, 0x4A );
  jpeg_encoder_emit_byte( enc, 0x46 );
  jpeg_encoder_emit_byte( enc, 0x49 );
  jpeg_encoder_emit_byte( enc, 0x46 );

  jpeg_encoder_emit_byte( enc, 0 );
  jpeg_encoder_emit_byte( enc, 1 );      /* Major version */
  jpeg_encoder_emit_byte( enc, 1 );      /* Minor version */
  jpeg_encoder_emit_byte( enc, 0 );      /* Density unit */
  jpeg_encoder_emit_word( enc, 1 );
  jpeg_encoder_emit_word( enc, 1 );
  jpeg_encoder_emit_byte( enc, 0 );      /* No thumbnail image */
  jpeg_encoder_emit_byte( enc, 0 );
}

/*****************************************************************************/

// Emit quantization tables
static void jpeg_encoder_emit_dqt(struct jpeg_encoder *enc) {
  int n = (enc->m_num_components == 3) ? 2 : 1;
  for( int i = 0; i < n; i++ )
  {
    jpeg_encoder_emit_marker( enc, M_DQT );
    jpeg_encoder_emit_word( enc, 64 + 1 + 2 );
    jpeg_encoder_emit_byte( enc, (uint8_t)i );
    for( int j = 0; j < 64; j++ )
    {
      jpeg_encoder_emit_byte( enc,
          (uint8_t)(enc->m_huff[i].m_quantization_table[j]) );
    }
  }
}

/*****************************************************************************/

// Emit start of frame marker
static void jpeg_encoder_emit_sof(struct jpeg_encoder *enc) {
  jpeg_encoder_emit_marker( enc, M_SOF0 );  /* baseline */
  jpeg_encoder_emit_word( enc, 3 * enc->m_num_components + 2 + 5 + 1 );
  jpeg_encoder_emit_byte( enc, 8 );    /* precision */
  jpeg_encoder_emit_word( enc, (uint32_t)enc->m_y );
  jpeg_encoder_emit_word( enc, (uint32_t)enc->m_x );
  jpeg_encoder_emit_byte( enc, enc->m_num_components );
  for( int i = 0; i < enc->m_num_components; i++ )
  {
    /* component ID */
    jpeg_encoder_emit_byte( enc, (uint8_t)(i + 1) );
    /* h and v sampling */
    jpeg_encoder_emit_byte( enc, (uint8_t)
        ((enc->m_comp[i].m_h_samp << 4) + enc->m_comp[i].m_v_samp) );
    /* quant. table num */
    jpeg_encoder_emit_byte( enc, i > 0 );
  }
}

/*****************************************************************************/

// Emit Huffman table.
static void jpeg_encoder_emit_dht(
        struct jpeg_encoder *enc,
        uint8_t *bits,
        uint8_t *val,
        int index,
        gboolean ac_flag) {
  jpeg_encoder_emit_marker( enc, M_DHT );

  int length = 0;
  for( int i = 1; i <= 16; i++ )
  {
    length += bits[i];
  }

  jpeg_encoder_emit_word( enc, (uint32_t)(length + 2 + 1 + 16) );
  jpeg_encoder_emit_byte( enc, (uint8_t)(index + (ac_flag << 4)) );

  for( int i = 1; i <= 16; i++ )
  {
    jpeg_encoder_emit_byte( enc, bits[i] );
  }

  for( int i = 0; i < length; i++ )
  {
    jpeg_encoder_emit_byte( enc, val[i] );
  }
}

/*****************************************************************************/

// Emit all Huffman tables.
static void jpeg_encoder_emit_dhts(struct jpeg_encoder *enc) {
  jpeg_encoder_emit_dht(
      enc, enc->m_huff[0].dc.m_bits, enc->m_huff[0].dc.m_val, 0, FALSE );
  jpeg_encoder_emit_dht(
      enc, enc->m_huff[0].ac.m_bits, enc->m_huff[0].ac.m_val, 0, TRUE );

  if( enc->m_num_components == 3 )
  {
    jpeg_encoder_emit_dht(
        enc, enc->m_huff[1].dc.m_bits, enc->m_huff[1].dc.m_val, 1, FALSE );
    jpeg_encoder_emit_dht(
        enc, enc->m_huff[1].ac.m_bits, enc->m_huff[1].ac.m_val, 1, TRUE );
  }
}

/*****************************************************************************/

// emit start of scan
static void jpeg_encoder_emit_sos(struct jpeg_encoder *enc) {
  jpeg_encoder_emit_marker( enc, M_SOS );
  jpeg_encoder_emit_word( enc, 2 * enc->m_num_components + 2 + 1 + 3 );
  jpeg_encoder_emit_byte( enc, enc->m_num_components );
  for( int i = 0; i < enc->m_num_components; i++ )
  {
    jpeg_encoder_emit_byte( enc, (uint8_t)(i + 1) );
    if( i == 0 )
    {
      jpeg_encoder_emit_byte( enc, (0 << 4) + 0 );
    }
    else
    {
      jpeg_encoder_emit_byte( enc, (1 << 4) + 1 );
    }
  }
  jpeg_encoder_emit_byte( enc, 0 );     /* spectral selection */
  jpeg_encoder_emit_byte( enc, 63 );
  jpeg_encoder_emit_byte( enc, 0 );
}

/*****************************************************************************/

// Emit all markers at beginning of image file.
static void jpeg_encoder_emit_start_markers(struct jpeg_encoder *enc) {
  jpeg_encoder_emit_marker( enc, M_SOI );
  jpeg_encoder_emit_jfif_app0( enc );
  jpeg_encoder_emit_dqt( enc );
  jpeg_encoder_emit_sof( enc );
  jpeg_encoder_emit_dhts( enc );
  jpeg_encoder_emit_sos( enc );
}

/*****************************************************************************/

// Quantization table generation.
static void jpeg_encoder_compute_quant_table(
        struct jpeg_encoder *enc,
        int32_t *pDst,
        int16_t *pSrc) {
  float q;
  if( enc->m_params.m_quality < 50 )
  {
    q = 5000.0f / enc->m_params.m_quality;
  }
  else
  {
    q = 200.0f - enc->m_params.m_quality * 2.0f;
  }
  for( int i = 0; i < 64; i++ )
  {
    int32_t j = pSrc[i];
    j = ( j * (int32_t)q + 50L ) / 100L;
    pDst[i] = JPEG_MIN( JPEG_MAX(j, 1), 1024 / 3 );
  }

  // DC quantized worse than 8 makes overall quality fall off the cliff
  if( pDst[0] > 8 )  pDst[0] = ( pDst[0] + 8 * 3 ) / 4;
  if( pDst[1] > 24 ) pDst[1] = ( pDst[1] + 24 ) / 2;
  if( pDst[2] > 24 ) pDst[2] = ( pDst[2] + 24 ) / 2;
}

/*****************************************************************************/

static void jpeg_encoder_reset_last_dc(struct jpeg_encoder *enc) {
  enc->m_bit_buffer = 0;
  enc->m_bits_in = 0;
  enc->m_comp[0].m_last_dc_val = 0;
  enc->m_comp[1].m_last_dc_val = 0;
  enc->m_comp[2].m_last_dc_val = 0;
}

/*****************************************************************************/

static void jpeg_encoder_compute_huffman_tables(struct jpeg_encoder *enc) {
  huffman_table_optimize( &(enc->m_huff[0].dc), DC_LUM_CODES );
  huffman_table_compute(  &(enc->m_huff[0].dc) );

  huffman_table_optimize( &(enc->m_huff[0].ac), AC_LUM_CODES );
  huffman_table_compute(  &(enc->m_huff[0].ac) );

  if( enc->m_num_components > 1 )
  {
    huffman_table_optimize( &(enc->m_huff[1].dc), DC_CHROMA_CODES );
    huffman_table_compute(  &(enc->m_huff[1].dc) );

    huffman_table_optimize( &(enc->m_huff[1].ac), AC_CHROMA_CODES );
    huffman_table_compute(  &(enc->m_huff[1].ac) );
  }
}

/*****************************************************************************/

static gboolean jpeg_encoder_jpg_open(
        struct jpeg_encoder *enc,
        int p_x_res,
        int p_y_res) {
  enc->m_num_components = 3;
  switch( enc->m_params.m_subsampling  )
  {
    case Y_ONLY:
      {
        enc->m_num_components = 1;
        enc->m_comp[0].m_h_samp = 1;
        enc->m_comp[0].m_v_samp = 1;
        enc->m_mcu_w = 8;
        enc->m_mcu_h = 8;
        break;
      }
    case H1V1:
      {
        enc->m_comp[0].m_h_samp = 1;
        enc->m_comp[0].m_v_samp = 1;
        enc->m_comp[1].m_h_samp = 1;
        enc->m_comp[1].m_v_samp = 1;
        enc->m_comp[2].m_h_samp = 1;
        enc->m_comp[2].m_v_samp = 1;
        enc->m_mcu_w = 8;
        enc->m_mcu_h = 8;
        break;
      }
    case H2V1:
      {
        enc->m_comp[0].m_h_samp = 2;
        enc->m_comp[0].m_v_samp = 1;
        enc->m_comp[1].m_h_samp = 1;
        enc->m_comp[1].m_v_samp = 1;
        enc->m_comp[2].m_h_samp = 1;
        enc->m_comp[2].m_v_samp = 1;
        enc->m_mcu_w = 16;
        enc->m_mcu_h = 8;
        break;
      }
    case H2V2:
      {
        enc->m_comp[0].m_h_samp = 2;
        enc->m_comp[0].m_v_samp = 2;
        enc->m_comp[1].m_h_samp = 1;
        enc->m_comp[1].m_v_samp = 1;
        enc->m_comp[2].m_h_samp = 1;
        enc->m_comp[2].m_v_samp = 1;
        enc->m_mcu_w = 16;
        enc->m_mcu_h = 16;
      }
  }

  enc->m_x = p_x_res;
  enc->m_y = p_y_res;
  enc->m_image[2].m_x = enc->m_image[1].m_x = enc->m_image[0].m_x =
    (enc->m_x + enc->m_mcu_w - 1) & (~(enc->m_mcu_w - 1));
  enc->m_image[2].m_y = enc->m_image[1].m_y = enc->m_image[0].m_y =
    (enc->m_y + enc->m_mcu_h - 1) & (~(enc->m_mcu_h - 1));

  for( int c = 0; c < enc->m_num_components; c++ )
  {
    image_init( &(enc->m_image[c]) );
  }

  memset( (void *)&(enc->m_huff), 0, sizeof(enc->m_huff) );
  jpeg_encoder_compute_quant_table(
      enc, enc->m_huff[0].m_quantization_table, s_std_lum_quant );
  jpeg_encoder_compute_quant_table(
      enc, enc->m_huff[1].m_quantization_table,
      enc->m_params.m_no_chroma_discrim_flag ?
      s_std_lum_quant : s_std_croma_quant );

  enc->m_out_buf_left = JPEG_OUT_BUF_SIZE;
  enc->m_pOut_buf = enc->m_out_buf;

  jpeg_encoder_reset_last_dc( enc );

  return( enc->m_all_stream_writes_succeeded );
}

/*****************************************************************************/

static inline dctq_t round_to_zero(const dct_t j, const int32_t quant) {
  if( j < 0 )
  {
    dctq_t jtmp = (dctq_t)(-j) + (dctq_t)(quant >> 1);
    return( (jtmp < quant) ? 0 : (dctq_t)( -(jtmp / quant) ) );
  }
  else
  {
    dctq_t jtmp = (dctq_t)j + (dctq_t)(quant >> 1);
    return( (jtmp < quant) ? 0 : (dctq_t)((jtmp / quant)) );
  }
}

/*****************************************************************************/

static void jpeg_encoder_quantize_pixels(
        struct jpeg_encoder *enc,
        dct_t *pSrc,
        dctq_t *pDst,
        const int32_t *quant) {
  dct( pSrc );
  for( int i = 0; i < 64; i++ )
  {
    pDst[i] = round_to_zero( pSrc[s_zag[i]], quant[i] );
  }
}

/*****************************************************************************/

static void jpeg_encoder_flush_output_buffer(struct jpeg_encoder *enc) {
  if( enc->m_out_buf_left != JPEG_OUT_BUF_SIZE )
  {
    int len = JPEG_OUT_BUF_SIZE - (int)enc->m_out_buf_left;
    gboolean status =
      jpeg_encoder_put_buf( enc->m_out_buf, len, enc->m_pStream );
    enc->m_all_stream_writes_succeeded =
      enc->m_all_stream_writes_succeeded && status;
  }

  enc->m_pOut_buf = enc->m_out_buf;
  enc->m_out_buf_left = JPEG_OUT_BUF_SIZE;
}

/*****************************************************************************/

static inline uint32_t bit_count(int temp1) {
  if( temp1 < 0 )
  {
    temp1 = -temp1;
  }

  uint32_t nbits = 0;
  while( temp1 )
  {
    nbits++;
    temp1 >>= 1;
  }

  return( nbits );
}

/*****************************************************************************/

/* TODO may be optimized */
#define JPEG_PUT_BYTE(c) \
{ \
  *(enc->m_pOut_buf)++ = (c); \
  if( --(enc->m_out_buf_left) == 0 ) \
  jpeg_encoder_flush_output_buffer( enc ); \
}

static void jpeg_encoder_put_bits(
        struct jpeg_encoder *enc,
        uint32_t bits,
        uint32_t len) {
  uint8_t c;

  uint32_t shift = 24 - (enc->m_bits_in += len);
  enc->m_bit_buffer |= ( (uint32_t)bits << shift );
  while( enc->m_bits_in >= 8 )
  {
    c = (uint8_t)( (enc->m_bit_buffer >> 16) & 0xFF );
    JPEG_PUT_BYTE( c )
      if( c == 0xFF )
      {
        JPEG_PUT_BYTE( 0 )
      }

    enc->m_bit_buffer <<= 8;
    enc->m_bits_in -= 8;
  }
}

/*****************************************************************************/

static void jpeg_encoder_put_signed_int_bits(
        struct jpeg_encoder *enc,
        int num,
        uint32_t len) {
  if( num < 0 )
  {
    num--;
  }
  jpeg_encoder_put_bits( enc, num & ((1 << len) - 1), len );
}

/*****************************************************************************/

static void jpeg_encoder_code_block(
        struct jpeg_encoder *enc,
        dctq_t *src,
        struct huffman_dcac *huff,
        struct component *comp,
        gboolean write) {
  const int dc_delta  = src[0] - comp->m_last_dc_val;
  comp->m_last_dc_val = src[0];

  uint32_t nbits = bit_count( dc_delta );

  if( write )
  {
    jpeg_encoder_put_bits(
        enc, huff->dc.m_codes[nbits], huff->dc.m_code_sizes[nbits] );
    jpeg_encoder_put_signed_int_bits( enc, dc_delta, nbits );
  }
  else
  {
    huff->dc.m_count[nbits]++;
  }

  int run_len = 0;

  for( int i = 1; i < 64; i++ )
  {
    const dctq_t ac_val = src[i];

    if( ac_val == 0 )
    {
      run_len++;
    }
    else
    {
      while( run_len >= 16 )
      {
        if( write )
        {
          jpeg_encoder_put_bits(
              enc, huff->ac.m_codes[0xF0], huff->ac.m_code_sizes[0xF0] );
        }
        else
        {
          huff->ac.m_count[0xF0]++;
        }
        run_len -= 16;
      }

      nbits = bit_count( ac_val );
      const int code = ( run_len << 4 ) + (int)nbits;

      if( write )
      {
        jpeg_encoder_put_bits(
            enc, huff->ac.m_codes[code], huff->ac.m_code_sizes[code] );
        jpeg_encoder_put_signed_int_bits( enc, ac_val, nbits);
      }
      else
      {
        huff->ac.m_count[code]++;
      }

      run_len = 0;
    }
  }

  if( run_len )
  {
    if( write )
    {
      jpeg_encoder_put_bits(
          enc, huff->ac.m_codes[0], huff->ac.m_code_sizes[0] );
    }
    else
    {
      huff->ac.m_count[0]++;
    }
  }
}

/*****************************************************************************/

static void jpeg_encoder_code_mcu_row(
        struct jpeg_encoder *enc,
        int y,
        gboolean write) {
  dctq_t *dctq;
  if( enc->m_num_components == 1 )
  {
    for( int x = 0; x < enc->m_x; x += enc->m_mcu_w )
    {
      dctq = image_get_dctq( &(enc->m_image[0]), x, y );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[0]), &(enc->m_comp[0]), write );
    }
  }
  else if(
      (enc->m_comp[0].m_h_samp == 1) &&
      (enc->m_comp[0].m_v_samp == 1) )
  {
    for( int x = 0; x < enc->m_x; x += enc->m_mcu_w )
    {
      dctq = image_get_dctq( &(enc->m_image[0]), x, y );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[0]), &(enc->m_comp[0]), write );
      dctq = image_get_dctq( &(enc->m_image[1]), x, y );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[1]), &(enc->m_comp[1]), write );
      dctq = image_get_dctq( &(enc->m_image[2]), x, y );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[1]), &(enc->m_comp[2]), write );
    }
  }
  else if(
      (enc->m_comp[0].m_h_samp == 2) &&
      (enc->m_comp[0].m_v_samp == 1) )
  {
    for( int x = 0; x < enc->m_x; x += enc->m_mcu_w )
    {
      dctq = image_get_dctq( &(enc->m_image[0]), x, y );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[0]), &(enc->m_comp[0]), write );
      dctq = image_get_dctq( &(enc->m_image[0]), x+8, y );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[0]), &(enc->m_comp[0]), write );
      dctq = image_get_dctq( &(enc->m_image[1]), x/2, y );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[1]), &(enc->m_comp[1]), write );
      dctq = image_get_dctq( &(enc->m_image[2]), x/2, y );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[1]), &(enc->m_comp[2]), write );
    }
  }
  else if(
      (enc->m_comp[0].m_h_samp == 2) &&
      (enc->m_comp[0].m_v_samp == 2) )
  {
    for( int x = 0; x < enc->m_x; x += enc->m_mcu_w )
    {
      dctq = image_get_dctq( &(enc->m_image[0]), x, y );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[0]), &(enc->m_comp[0]), write );
      dctq = image_get_dctq( &(enc->m_image[0]), x+8, y );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[0]), &(enc->m_comp[0]), write );
      dctq = image_get_dctq( &(enc->m_image[0]), x, y+8 );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[0]), &(enc->m_comp[0]), write );
      dctq = image_get_dctq( &(enc->m_image[0]), x+8, y+8 );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[0]), &(enc->m_comp[0]), write );
      dctq = image_get_dctq( &(enc->m_image[1]), x/2, y/2 );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[1]), &(enc->m_comp[1]), write );
      dctq = image_get_dctq( &(enc->m_image[2]), x/2, y/2 );
      jpeg_encoder_code_block(
          enc, dctq, &(enc->m_huff[1]), &(enc->m_comp[2]), write );
    }
  }
}

/*****************************************************************************/

static gboolean jpeg_encoder_emit_end_markers(struct jpeg_encoder *enc) {
  jpeg_encoder_put_bits( enc, 0x7F, 7 );
  jpeg_encoder_flush_output_buffer( enc );
  jpeg_encoder_emit_marker( enc, M_EOI );
  return( enc->m_all_stream_writes_succeeded );
}

/*****************************************************************************/

static gboolean jpeg_encoder_compress_image(struct jpeg_encoder *enc) {
  for( int c = 0; c < enc->m_num_components; c++ )
  {
    for( int y = 0; y < enc->m_image[c].m_y; y+= 8 )
    {
      for( int x = 0; x < enc->m_image[c].m_x; x += 8 )
      {
        dct_t sample[64];
        image_load_block( &(enc->m_image[c]), sample, x, y );
        dctq_t *dctq = image_get_dctq( &(enc->m_image[c]), x, y );
        jpeg_encoder_quantize_pixels(
            enc, sample, dctq, enc->m_huff[c > 0].m_quantization_table );
      }
    }
  }

  for( int y = 0; y < enc->m_y; y += enc->m_mcu_h )
  {
    jpeg_encoder_code_mcu_row( enc, y, FALSE );
  }
  jpeg_encoder_compute_huffman_tables( enc );
  jpeg_encoder_reset_last_dc( enc );
  jpeg_encoder_emit_start_markers( enc );

  for( int y = 0; y < enc->m_y; y+= enc->m_mcu_h )
  {
    if( !enc->m_all_stream_writes_succeeded )
    {
      return( FALSE );
    }
    jpeg_encoder_code_mcu_row( enc, y, TRUE );
  }

  return( jpeg_encoder_emit_end_markers(enc) );
}

/*****************************************************************************/

static void jpeg_encoder_load_mcu_Y(
        struct jpeg_encoder *enc,
        const uint8_t *pSrc,
        int width,
        int bpp,
        int y) {
  if( bpp == 4 )
  {
    RGB_to_Y( &(enc->m_image[0]), pSrc, RGBA, width, y );
  }
  else if( bpp == 3 )
  {
    RGB_to_Y( &(enc->m_image[0]), pSrc, RGB, width, y );
  }
  else
    for( int x = 0; x < width; x++ )
    {
      image_set_px( &(enc->m_image[0]), pSrc[x] - 128.0f, x, y );
    }

  // Possibly duplicate pixels at end of
  // scanline if not a multiple of 8 or 16
  const float lastpx =
    image_get_px( &(enc->m_image[0]), width - 1, y );
  for( int x = width; x < enc->m_image[0].m_x; x++ )
  {
    image_set_px( &(enc->m_image[0]), lastpx, x, y );
  }
}

/*****************************************************************************/

static void jpeg_encoder_load_mcu_YCC(
        struct jpeg_encoder *enc,
        const uint8_t *pSrc,
        int width,
        int bpp,
        int y) {
  if( bpp == 4 )
  {
    RGB_to_YCC( enc->m_image, pSrc, RGBA, width, y );
  }
  else if( bpp == 3 )
  {
    RGB_to_YCC( enc->m_image, pSrc, RGB, width, y );
  }
  else
  {
    Y_to_YCC( enc->m_image, pSrc, width, y );
  }

  // Possibly duplicate pixels at end of
  // scanline if not a multiple of 8 or 16
  for( int c = 0; c < enc->m_num_components; c++ )
  {
    const float lastpx =
      image_get_px( &(enc->m_image[c]), width - 1, y );
    for( int x = width; x < enc->m_image[0].m_x; x++ )
    {
      image_set_px( &(enc->m_image[c]), lastpx, x, y );
    }
  }
}

/*****************************************************************************/

static void jpeg_encoder_clear(struct jpeg_encoder *enc) {
  enc->m_num_components = 0;
  enc->m_all_stream_writes_succeeded = TRUE;
}

/*****************************************************************************/

static void jpeg_encoder_deinit(struct jpeg_encoder *enc) {
  for( int c = 0; c < enc->m_num_components; c++ )
  {
    image_deinit( &(enc->m_image[c]) );
  }
  jpeg_encoder_clear( enc );
}

/*****************************************************************************/

static inline gboolean params_check(compression_params_t *parm) {
  if( (parm->m_quality < 1) || (parm->m_quality > 100) )
  {
    return( FALSE );
  }
  if( (uint32_t)parm->m_subsampling > (uint32_t)H2V2 )
  {
    return( FALSE );
  }
  return( TRUE );
}

/*****************************************************************************/

static gboolean jpeg_encoder_init(
        struct jpeg_encoder *enc,
        FILE *pStream,
        int width,
        int height,
        compression_params_t *comp_params) {
  //jpeg_encoder_deinit( enc );
  jpeg_encoder_clear( enc );

  if( !pStream ||
      width < 1 ||
      height < 1 ||
      !params_check(comp_params) )
  {
    return( FALSE );
  }

  enc->m_pStream = pStream;
  enc->m_params = *comp_params;

  return( jpeg_encoder_jpg_open(enc, width, height) );
}

/*****************************************************************************/

static gboolean jpeg_encoder_read_image(
        struct jpeg_encoder *enc,
        const uint8_t *image_data,
        int width,
        int height,
        int bpp) {
  if( (bpp != 1) && (bpp != 3) && (bpp != 4) )
  {
    return( FALSE );
  }

  for( int y = 0; y < height; y++ )
  {
    if( enc->m_num_components == 1 )
    {
      jpeg_encoder_load_mcu_Y(
          enc, image_data + width * y * bpp, width, bpp, y );
    }
    else
    {
      jpeg_encoder_load_mcu_YCC(
          enc, image_data + width * y * bpp, width, bpp, y );
    }
  }

  for( int c = 0; c < enc->m_num_components; c++ )
  {
    for( int y = height; y < enc->m_image[c].m_y; y++ )
    {
      for( int x = 0; x < enc->m_image[c].m_x; x++ )
      {
        float pix = image_get_px( &(enc->m_image[c]), x, y-1 );
        image_set_px( &(enc->m_image[c]), pix, x, y );
      }
    }
  }

  if( enc->m_comp[0].m_h_samp == 2 )
  {
    for( int c = 1; c < enc->m_num_components; c++ )
    {
      image_subsample( &(enc->m_image[c]),
          &(enc->m_image[0]), enc->m_comp[0].m_v_samp );
    }
  }

  // overflow white and black, making distortions overflow as well,
  // so distortions (ringing) will be clamped by the decoder
  if( enc->m_huff[0].m_quantization_table[0] > 2 )
  {
    for( int c = 0; c < enc->m_num_components; c++ )
    {
      for( int y = 0; y < enc->m_image[c].m_y; y++ )
      {
        for( int x = 0; x < enc->m_image[c].m_x; x++ )
        {
          float px = image_get_px( &(enc->m_image[c]), x, y );
          if( px <= -128.0f )
          {
            px -= enc->m_huff[0].m_quantization_table[0];
          }
          else if( px >= 128.0f )
          {
            px += enc->m_huff[0].m_quantization_table[0];
          }
          image_set_px( &(enc->m_image[c]), px, x, y );
        }
      }
    }
  }

  return( TRUE );
}

/*****************************************************************************/

/******* My own higher-level helper/wrapper functions *******/

/* jpeg_encoder_compression_parameters()
 *
 * Sets the JPEG compression parameters
 * to a compression_params_t struct
 */
gboolean jpeg_encoder_compression_parameters(
        compression_params_t *comp_params,
        float m_quality,
        enum subsampling_t m_subsampling,
        gboolean m_no_chroma_discrim_flag) {
  comp_params->m_quality = m_quality;
  comp_params->m_subsampling = m_subsampling;
  comp_params->m_no_chroma_discrim_flag = m_no_chroma_discrim_flag;
  return( params_check(comp_params) );
}

/*****************************************************************************/

/* jpeg_encoder_compress_image_to_file()
 *
 * Saves an image buffer to a JPEG-compressed file
 */
gboolean jpeg_encoder_compress_image_to_file(
        char *file_name,
        int width,
        int height,
        int num_channels,
        const uint8_t *pImage_data,
        compression_params_t *comp_params) {
  struct jpeg_encoder encoder;
  FILE *pFile = NULL;
  gboolean ret;

  ret = Open_File( &pFile, file_name, "w" );
  if( !ret )
  {
    return( FALSE );
  }

  ret = jpeg_encoder_init(
      &encoder, pFile, width, height, comp_params );
  if( !ret )
  {
    Show_Message( "Error: jpeg_encoder_init() failed", "red" );
    return( FALSE );
  }

  ret = jpeg_encoder_read_image(
      &encoder, pImage_data, width, height, num_channels );
  if( !ret )
  {
    Show_Message( "Error: jpeg_encoder_read_image() failed", "red" );
    return( FALSE );
  }

  ret = jpeg_encoder_compress_image( &encoder );
  if( !ret )
  {
    Show_Message( "Error: jpeg_encoder_compress_image() failed", "red" );
    return( FALSE );
  }

  jpeg_encoder_deinit( &encoder );
  fclose( pFile );

  return( TRUE );
}
