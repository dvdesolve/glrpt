/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 3 of
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

#include "met_jpg.h"

#include "../common/common.h"
#include "../common/shared.h"
#include "../glrpt/clahe.h"
#include "../glrpt/image.h"
#include "../glrpt/jpeg.h"
#include "../glrpt/utils.h"
#include "alib.h"
#include "dct.h"
#include "huffman.h"
#include "rectify_meteor.h"
#include "viterbi27.h"

#include <glib.h>

#include <math.h>
#include <stddef.h>
#include <stdint.h>

/*****************************************************************************/

#define MCU_PER_PACKET  14

/*****************************************************************************/

enum {
    GREYSCALE_CHAN = 1,
    COLORIZED_CHAN = 3
};

/*****************************************************************************/

static void Save_Images(int type);
static void Fill_Dqt_by_Q(int *dqt, int q);
static void Fill_Pix(double *img_dct, uint32_t apid, int mcu_id, int m);
static gboolean Progress_Image(uint32_t apid, int mcu_id, int pck_cnt);

/*****************************************************************************/

static int last_mcu  = -1;
static int cur_y     = 0;
static int last_y    = -1;
static int first_pck = 0;
static int prev_pck  = 0;

static const uint8_t standard_quantization_table[64] = {
    16,  11,  10,  16,  24,  40,  51,  61,
    12,  12,  14,  19,  26,  58,  60,  55,
    14,  13,  16,  24,  40,  57,  69,  56,
    14,  17,  22,  29,  51,  87,  80,  62,
    18,  22,  37,  56,  68, 109, 103,  77,
    24,  35,  55,  64,  81, 104, 113,  92,
    49,  64,  78,  87, 103, 121, 120, 101,
    72,  92,  95,  98, 112, 100, 103,  99
};

static const uint8_t zigzag[64] = {
    0,  1,  5,  6, 14, 15, 27, 28,
    2,  4,  7, 13, 16, 26, 29, 42,
    3,  8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

static const int dc_cat_off[12] = { 2, 3, 3, 3, 3, 3, 4, 5, 6, 7, 8, 9 };

/*****************************************************************************/

/* Save_Images()
 *
 * My addition, separated code that saves images
 */
static void Save_Images(int type) {
  char fname[MAX_FILE_NAME];
  uint32_t idx;

  /* Store APID images individually as PGM files */
  if( isFlagSet(IMAGE_OUT_SPLIT) )
  {
    for( idx = 0; idx < CHANNEL_IMAGE_NUM; idx++ )
    {
      /* Save channel images as raw PGM */
      if( isFlagSet(IMAGE_SAVE_PPGM) )
      {
        /* Save unprocessed image */
        fname[0] = '\0';
        if( type == IMAGE_RAW )
          File_Name( fname, idx, "-raw.pgm" );
        else
          File_Name( fname, idx, ".pgm" );
        Save_Image_Raw( fname, "P5",
            channel_image_width,
            channel_image_height,
            255, channel_image[idx] );
      }

      /* Save channel images as JPEG */
      if( isFlagSet(IMAGE_SAVE_JPEG) )
      {
        /* Set compression parameters */
        compression_params_t comp_params;
        gboolean ret = jpeg_encoder_compression_parameters(
            &comp_params, rc_data.jpeg_quality, Y_ONLY, TRUE );
        if( !ret )
          Show_Message( "Bad compression parameters", "red" );

        /* Save unprocessed image */
        fname[0] = '\0';
        if( type == IMAGE_RAW )
          File_Name( fname, idx, "-raw.jpg" );
        else
          File_Name( fname, idx, ".jpg" );
        Save_Image_JPEG( fname,
            (int)channel_image_width,
            (int)channel_image_height,
            GREYSCALE_CHAN,
            channel_image[idx],
            &comp_params );
      }

    } /* for( idx = 0; idx < CHANNEL_IMAGE_NUM; idx++ ) */
  } /* if( isFlagSet(IMAGE_OUT_SPLIT) ) */

  /* Create pseudo-color combo image */
  if( isFlagSet(IMAGE_OUT_COMBO) )
  {
    uint8_t *combo_image = NULL;
    mem_alloc( (void **)&combo_image, channel_image_size * 3 );
    Create_Combo_Image( combo_image );

    /* Save combo image as raw PGM */
    if( isFlagSet(IMAGE_SAVE_PPGM) )
    {
      /* Save unprocessed image */
      fname[0] = '\0';
      if( type == IMAGE_RAW )
        File_Name( fname, COMBO, "-raw.ppm" );
      else
        File_Name( fname, COMBO, ".ppm" );
      Save_Image_Raw( fname, "P6",
          channel_image_width,
          channel_image_height,
          255, combo_image );
    }

    /* Save combo image as JPEG */
    if( isFlagSet(IMAGE_SAVE_JPEG) )
    {
      /* Set compression parameters */
      compression_params_t comp_params;
      gboolean ret = jpeg_encoder_compression_parameters(
          &comp_params, rc_data.jpeg_quality, H2V2, FALSE );
      if( !ret ) Show_Message( "Bad compression parameters", "red" );

      /* Save unprocessed image */
      fname[0] = '\0';
      if( type == IMAGE_RAW )
        File_Name( fname, COMBO, "-raw.jpg" );
      else
        File_Name( fname, COMBO, ".jpg" );
      Save_Image_JPEG( fname,
          (int)channel_image_width,
          (int)channel_image_height,
          COLORIZED_CHAN, combo_image,
          &comp_params );
    }

    free_ptr( (void **)&combo_image );
  } /* if( isFlagSet(IMAGE_OUT_COMBO) ) */

}

/*****************************************************************************/

void Mj_Dump_Image(void) {
  uint32_t idx;

  /* Abort if no images successfully decoded */
  if( channel_image_size == 0 )
    return;

  /* My addition, process images when reception finished */
  if( isFlagClear(STATUS_RECEIVING) )
  {
    /* Save images in Raw state first, if enabled */
    if( isFlagSet(IMAGE_RAW) ) Save_Images( IMAGE_RAW );

    /* Process images if not already done */
    if( isFlagClear(IMAGES_PROCESSED) )
    {
      /* My addition, invert image (flip vertically) */
      if( isFlagSet(IMAGE_INVERT) )
      {
        for( idx = 0; idx < CHANNEL_IMAGE_NUM; idx++ )
          Flip_Image( channel_image[idx], (uint32_t)channel_image_size );
      }

      /* Rectify (stretch) images to correct scan distortion */
      if( isFlagSet(IMAGE_RECTIFY) && isFlagClear(IMAGES_RECTIFIED) )
        Rectify_Images();

      /* Normalize images if enabled */
      if( isFlagSet(IMAGE_NORMALIZE) )
      {
        for( idx = 0; idx < CHANNEL_IMAGE_NUM; idx++ )
        {
          /* Normalize (Equalize) histogram to cover full pixel value range */
          Normalize_Image( channel_image[idx],
              (uint32_t)channel_image_size, NORM_BLACK, MAX_WHITE );

          /* C.L.A.H.E. Normalization, see ../glrpt/clahe.c */
          if( isFlagSet(IMAGE_CLAHE) )
          {
            if( !CLAHE( channel_image[idx],
                  channel_image_width,
                  channel_image_height,
                  NORM_BLACK, MAX_WHITE,
                  REGIONS_X, REGIONS_Y,
                  NUM_GREYBINS, CLIP_LIMIT ) )
            {
              Show_Message(
                  "Failed to perform C.L.A.H.E.\n"\
                    "Image Contrast Enhancement", "red" );
            }
          } /* if( isFlagSet(IMAGE_CLAHE) ) */
        } /* for( idx = 0; idx < CHANNEL_IMAGE_NUM; idx++ ) */
      } /* if( isFlagSet(IMAGE_NORMALIZE) ) */

      SetFlag( IMAGES_PROCESSED );
    } /* if( isFlagClear(IMAGES_PROCESSED) ) */

    /* My addition, reset and display LRPT images when finished */
    if( isFlagClear(STATUS_RECEIVING) )
    {
      Display_Scaled_Image( NULL, 0, 0 );
      for( idx = 0; idx < CHANNEL_IMAGE_NUM; idx++ )
        Display_Scaled_Image(
            channel_image, rc_data.apid[idx],
            (int)channel_image_height );
    }

    /* Save processed images if enabled */
    if( isFlagSet(IMAGES_PROCESSED) )
      Save_Images( !IMAGE_RAW );

  } /* if( isFlagClear(STATUS_RECEIVING) ) */

}

/*****************************************************************************/

static void Fill_Dqt_by_Q(int *dqt, int q) {
  double f;
  int i;

  if( (q > 20 ) && (q < 50) )
    f = 5000.0 / (double)q;
  else
    f = 200.0 - 2.0 * (double)q;

  for( i = 0; i <= 63; i++ )
  {
    dqt[i] = (int)( round(f / 100.0 * (double)standard_quantization_table[i]) );
    if( dqt[i] < 1 ) dqt[i] = 1;
  }
}

/*****************************************************************************/

static void Fill_Pix(double *img_dct, uint32_t apid, int mcu_id, int m) {
  int i, j, t, x, y, off = 0, inv = 0;

  for( i = 0; i <= 63; i++ )
  {
    t = (int)( round(img_dct[i] + 128.0) );
    if( t < 0 )   t = 0;
    if( t > 255 ) t = 255;
    x = ( mcu_id + m ) * 8 + i % 8;
    y = cur_y + i / 8;
    off = x + y * METEOR_IMAGE_WIDTH;

    /* Invert image palette if APID matches */
    for( j = 0; j < 3; j++ )
      if( apid == rc_data.invert_palette[j] ) inv = 1;
    if( inv )
    {
      if( apid == rc_data.apid[RED] )
        channel_image[RED][off]   = 255 - (uint8_t)t;
      else if( apid == rc_data.apid[GREEN] )
        channel_image[GREEN][off] = 255 - (uint8_t)t;
      else if( apid == rc_data.apid[BLUE] )
        channel_image[BLUE][off]  = 255 - (uint8_t)t;
    }
    else /* Normal palette */
    {
      if( apid == rc_data.apid[RED] )
        channel_image[RED][off]   = (uint8_t)t;
      else if( apid == rc_data.apid[GREEN] )
        channel_image[GREEN][off] = (uint8_t)t;
      else if( apid == rc_data.apid[BLUE] )
        channel_image[BLUE][off]  = (uint8_t)t;
    }
  }
}

/*****************************************************************************/

static gboolean Progress_Image(uint32_t apid, int mcu_id, int pck_cnt) {
  static size_t prev_len = 0;
  size_t delta_len = 0, i, s;
  int j;

  if( (apid == 0) || (apid == 70) )
    return( FALSE );

  if( last_mcu == -1 )
  {
    if( mcu_id != 0 ) return( FALSE );
    prev_pck  = pck_cnt;
    first_pck = pck_cnt;
    if(  apid == 65 ) first_pck -= 14;
    if( (apid == 66) || (apid == 68) )
      first_pck -= 28;
    last_mcu = 0;
    cur_y = -1;
    prev_len = 0;
  }

  if( pck_cnt < prev_pck ) first_pck -= 16384;
  prev_pck = pck_cnt;

  cur_y = 8 * ( (pck_cnt - first_pck) / 43 );
  if( cur_y > last_y )
  {
    channel_image_height = (uint32_t)( cur_y + 8 );
    channel_image_size   = (size_t)
      ( channel_image_width * channel_image_height );
    for( i = 0; i < CHANNEL_IMAGE_NUM; i++ )
      mem_realloc( (void **)&channel_image[i], channel_image_size );

    /* Clear new allocation */
    delta_len = channel_image_size - prev_len;
    for( i = 0; i < delta_len; i++ )
    {
      s = i + prev_len;
      for( j = 0; j < CHANNEL_IMAGE_NUM; j++ )
        channel_image[j][s] = 0;
    }

    prev_len = channel_image_size;
  }
  last_y = cur_y;

  return( TRUE );
}

/*****************************************************************************/

void Mj_Dec_Mcus(
        uint8_t *p,
        uint32_t apid,
        int pck_cnt,
        int mcu_id,
        uint8_t q) {
  bit_io_rec_t b;
  int i, m;
  uint16_t k, n;
  double prev_dc;
  int dc_cat, ac;
  double dct[64];
  double zdct[64];
  double img_dct[64];
  int dqt[64];
  int ac_run, ac_size, ac_len;

  b.p = p;
  b.pos = 0;

  if( !Progress_Image(apid, mcu_id, pck_cnt) )
    return;

  Fill_Dqt_by_Q( dqt, q );

  prev_dc = 0;
  m = 0;
  while( m < MCU_PER_PACKET )
  {
    dc_cat = Get_DC( (uint16_t)(Bio_Peek_n_Bits(&b, 16)) );
    if( dc_cat == -1 )
    {
      Show_Message( "Bad DC huffman code!", "red" );
      return;
    }
    Bio_Advance_n_Bits( &b, dc_cat_off[dc_cat] );
    n = (uint16_t)(Bio_Fetch_n_Bits( &b, dc_cat ));

    zdct[0] = Map_Range( dc_cat, n ) + prev_dc;
    prev_dc = zdct[0];

    k = 1;
    while( k < 64 )
    {
      ac = Get_AC( (uint16_t)(Bio_Peek_n_Bits(&b, 16)) );
      if( ac == -1 )
      {
        Show_Message( "Bad DC huffman code!", "red" );
        return;
      }
      ac_len  = ac_table[ac].len;
      ac_size = ac_table[ac].size;
      ac_run  = ac_table[ac].run;
      Bio_Advance_n_Bits( &b, ac_len );

      if( (ac_run == 0) && (ac_size == 0) )
      {
        for( i = k; i <= 63; i++ ) zdct[i] = 0;
        break;
      }

      for( i = 0; i < ac_run; i++ )
      {
        zdct[k] = 0;
        k++;
      }

      if( ac_size != 0 )
      {
        n = (uint16_t)(Bio_Fetch_n_Bits( &b, ac_size ));
        zdct[k] = Map_Range( ac_size, n );
        k++;
      }
      else if( ac_run == 15 )
      {
        zdct[k] = 0;
        k++;
      }
    }

    for( i = 0; i <= 63; i++ )
      dct[i] = zdct[ zigzag[i] ] * dqt[i];

    Flt_Idct_8x8( img_dct, dct );
    Fill_Pix( img_dct, apid, mcu_id, m );
    m++;
  }

  /* My addition, incrementally display LRPT images */
  Display_Scaled_Image( channel_image, apid, cur_y );
}

/*****************************************************************************/

void Mj_Init(void) {
  Default_Huffman_Table();
  last_mcu  = -1;
  cur_y     = 0;
  last_y    = -1;
  first_pck = 0;
  prev_pck  = 0;
}
