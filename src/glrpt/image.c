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

#include "image.h"

#include "callback_func.h"
#include "../common/common.h"
#include "../common/shared.h"
#include "utils.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#include <stddef.h>
#include <stdint.h>

/*------------------------------------------------------------------------*/

/*  Normalize_Image()
 *
 *  Does histogram (linear) normalization of a pgm (P5) image file
 */

  void
Normalize_Image(
    uint8_t *image_buffer,
    uint32_t image_size,
    uint8_t range_low,
    uint8_t range_high )
{
  uint32_t
    hist[MAX_WHITE+1],  /* Intensity histogram of pgm image file  */
    pixel_cnt,          /* Total pixels counter for cut-off point */
    black_cutoff,       /* Count of pixels for black cutoff value */
    white_cutoff,       /* Count of pixels for white cutoff value */
    idx;

  uint8_t
    pixel_val_in,       /* Used for calculating normalized pixels */
    black_min_in,       /* Black cut-off pixel intensity value */
    white_max_in,       /* White cut-off pixel intensity value */
    val_range_in,       /* Range of intensity values in input image  */
    val_range_out;      /* Range of intensity values in output image */


  /* Abort for "empty" image buffers */
  if( image_size == 0 )
  {
    Show_Message(
        "Image file seems empty\n"\
          "Normalization not performed", "red" );
    Error_Dialog();
    return;
  }

  /* Clear histogram */
  for( idx = 0; idx <= MAX_WHITE; idx++ )
    hist[ idx ] = 0;

  /* Build image intensity histogram */
  for( idx = 0; idx < image_size; idx++ )
    hist[ image_buffer[idx] ]++;

  /* Determine black/white cut-off counts */
  black_cutoff = (image_size * BLACK_CUT_OFF) / 100;
  white_cutoff = (image_size * WHITE_CUT_OFF) / 100;

  /* Find black cut-off intensity value. Values below
   * MIN_BLACK are ignored to leave behind the black stripes
   * that seem to be sent by the satellite occasionally */
  pixel_cnt = 0;
  for( black_min_in = MIN_BLACK; black_min_in != MAX_WHITE; black_min_in++ )
  {
    pixel_cnt += hist[ black_min_in ];
    if( pixel_cnt >= black_cutoff ) break;
  }

  /* Find white cut-off intensity value */
  pixel_cnt = 0;
  for( white_max_in = MAX_WHITE; white_max_in != 0; white_max_in-- )
  {
    pixel_cnt += hist[ white_max_in ];
    if( pixel_cnt >= white_cutoff ) break;
  }

  /* Rescale pixels in image for required intensity range */
  val_range_in = white_max_in - black_min_in;
  if( val_range_in == 0 )
  {
    Show_Message(
        "Image seems flat\n"\
          "Normalization not performed", "red" );
    Error_Dialog();
    return;
  }

  /* Perform histogram normalization on images */
  Show_Message( "Performing Histogram Normalization", "black" );
  val_range_out = range_high - range_low;
  for( pixel_cnt = 0; pixel_cnt < image_size; pixel_cnt++ )
  {
    /* Input image pixel values relative to input black cut off.
     * Clamp pixel values within black and white cut off values */
    pixel_val_in  = (uint8_t)
      iClamp( image_buffer[pixel_cnt], black_min_in, white_max_in );
    pixel_val_in -= black_min_in;

    /* Normalized pixel values are scaled according to the ratio
     * of required pixel value range to input pixel value range */
    image_buffer[ pixel_cnt ] =
      range_low + ( pixel_val_in * val_range_out ) / val_range_in;
  }

} /* End of Normalize_Image() */

/*------------------------------------------------------------------------*/

/*  Flip_Image()
 *
 *  Flips a pgm (P5) image by 180 degrees
 */

  void
Flip_Image( uint8_t *image_buffer, uint32_t image_size )
{
  uint32_t idx; /* Index for loops etc */

  uint8_t
    *idx_temp,  /* Buffer location to be saved in temp    */
    *idx_swap;  /* Buffer location to be swaped with temp */

  /* Holds a pixel value temporarily */
  uint8_t temp;

  /* Abort for "empty" image buffers */
  if( image_size == 0 )
  {
    Show_Message(
        "Image file empty\n"\
          "Rotation not performed", "red" );
    Error_Dialog();
    return;
  }

  /* Rotate image 180 degrees */
  Show_Message( "Rotating Image by 180 degrees", "black" );
  for( idx = 0; idx < image_size / 2; idx++ )
  {
    idx_temp = image_buffer + idx;
    idx_swap = image_buffer - 1 + image_size - idx;
    temp = *( idx_temp );
    *( idx_temp ) = *( idx_swap );
    *( idx_swap ) = temp;
  }

} /* End of Flip_Image() */

/*------------------------------------------------------------------------*/

/* Display_Scaled_Image
 *
 * Scales an LRPT image horizontal line by the scale
 * factor and stores the result in the image pixbuf
 */
  void
Display_Scaled_Image(
    uint8_t *chan_image[],
    uint32_t apid,
    int current_y )
{
  int chn, idx, idy, cnt, scale;
  int scaled_width, scaled_x, scaled_idx;
  static int
    scaled_y[CHANNEL_IMAGE_NUM] = { 0, 0, 0 },
    last_y  [CHANNEL_IMAGE_NUM] = { 0, 0, 0 };
  uint16_t *pix_val = NULL;
  guchar *pixel, val;


  /* Signal to reset indices for new images */
  if( current_y == 0 )
  {
    for( cnt = 0; cnt < CHANNEL_IMAGE_NUM; cnt++ )
    {
      scaled_y[cnt] = 0;
      last_y[cnt]   = 0;
    }

    /* Fill pixbuf with background color */
    gdk_pixbuf_fill( scaled_image_pixbuf, 0xaaaaaaff );

    return;
  }

  /* Calculate scale factor for rectified images */
  scale = (int)rc_data.image_scale;
  if( isFlagSet(IMAGES_RECTIFIED) )
  {
    scaled_width = METEOR_IMAGE_WIDTH / scale;
    scale = (int)channel_image_width / scaled_width + 1;
  }

  /* Just in case the unscaled image height is too much */
  if( (current_y / scale) > scaled_image_height )
    current_y = scaled_image_height * scale;

  /* Find the channel image buffer for the given apid */
  for( chn = 0; chn < CHANNEL_IMAGE_NUM; chn++ )
    if( rc_data.apid[chn] == apid ) break;
  if( chn == CHANNEL_IMAGE_NUM ) return;

  /* Abort if channel image vertical size not enough */
  if( (current_y - last_y[chn]) < scale )
    return;

  /* Length of pixel values buffer */
  scaled_width = (int)channel_image_width / scale;

  /* Allocate pixel values buffer and clear */
  size_t siz = (size_t)scaled_width * sizeof(uint16_t);
  mem_alloc( (void **)&pix_val, siz );

  /* Keep scaling image while image size is enough */
  while( (current_y - last_y[chn]) >= scale )
  {
    /* Clear line buffer for next summation */
    bzero( pix_val, siz );

    /* Index to channel image to start using pixel values */
    idx = last_y[chn] * (int)channel_image_width;

    /* Summate (scale * scale) pixel values from the channel image */
    for( idy = 0; idy < scale; idy++ )
    {
      for( scaled_x = 0; scaled_x < scaled_width; scaled_x++ )
      {
        for( cnt = 0; cnt < scale; cnt++ )
          pix_val[scaled_x] += (uint16_t)chan_image[chn][idx++];
      }
      last_y[chn]++;
    }

    /* Fill scaled image buffer with scaled summed pixel values */
    int y =
      scaled_y[chn] * scaled_image_rowstride +
      (chn * scaled_width + chn) * scaled_image_n_channels;
    for( scaled_x = 0; scaled_x < scaled_width; scaled_x++ )
    {
      scaled_idx = scaled_x * scaled_image_n_channels + y;
      pixel = &scaled_image_pixel_buf[scaled_idx];
      val = (guchar)(pix_val[scaled_x] / (uint16_t)scale / (uint16_t)scale);
      pixel[0] = val;
      pixel[1] = val;
      pixel[2] = val;
    }

    /* Draw a vertical white line between images */
    scaled_idx = scaled_x * scaled_image_n_channels + y;
    pixel = &scaled_image_pixel_buf[scaled_idx];
    pixel[0] = 0xff;
    pixel[1] = 0xff;
    pixel[2] = 0xff;

    /* Go down the scaled image buffer */
    scaled_y[chn]++;

  } /* while( (current_y - last_y) >= rc_data.image_scale ) */
  free_ptr( (void **)&pix_val );

  /* Set lrpt image from pixbuff */
  gtk_image_set_from_pixbuf( GTK_IMAGE(lrpt_image), scaled_image_pixbuf );

} /* Display_Scaled_Image() */

/*------------------------------------------------------------------------*/

/* Create_Combo_Image()
 *
 * Combines channel images into one combined pseudo-color image.
 * If enabled, it performs some speculative enhancement of watery
 * areas and clouds.
 */
  void
Create_Combo_Image( uint8_t *combo_image )
{
  /* Color channels are 0 = red, 1 = green, 2 = blue
   * but it all depends on the APID options in glrptrc */
  uint32_t idx = 0, cnt;
  uint8_t range_red, range_green, range_blue;
  uint8_t
    red   = rc_data.color_channel[RED],
    green = rc_data.color_channel[GREEN],
    blue  = rc_data.color_channel[BLUE];

    /* Perform speculative enhancement of watery areas and clouds */
    if( isFlagSet(IMAGE_COLORIZE) )
    {
      /* The Red channel image from the Meteor M2 satellite seems
       * to have some excess luminance after Normalization so here
       * the pixel value range is reduced to that specified in the
       * ~/glrpt/glrptrc configuration file */
      range_red =
        rc_data.norm_range[RED][NORM_RANGE_WHITE] -
        rc_data.norm_range[RED][NORM_RANGE_BLACK];

      /* The Blue channel image from the Meteor M2 satellite looses
       * pixel values (luminance) in the watery areas (seas and lakes)
       * after Normalization. Here the pixel value range in the dark
       * areas is enhanced according to values specified in the
       * ~/glrpt/glrptrc configuration file */
      range_blue = rc_data.colorize_blue_max - rc_data.colorize_blue_min;

      for( cnt = 0; cnt < channel_image_size; cnt++ )
      {
        /* Progressively raise the value of blue channel
         * pixels in the dark areas to counteract the
         * effects of histogram equalization, which darkens
         * the parts of the image that are watery areas */
        if( isFlagClear(IMAGE_COLORIZED) )
        {
          if( channel_image[blue][cnt] < rc_data.colorize_blue_min )
          {
            channel_image[blue][cnt] =
              rc_data.colorize_blue_min  +
              ( channel_image[blue][cnt] * range_blue ) /
              rc_data.colorize_blue_max;
          }

          SetFlag( IMAGE_COLORIZED );
        }

        /* Colorize cloudy areas white pseudocolor. This helps
         * because the red channel does not render clouds right */
        if( channel_image[blue][cnt] > rc_data.clouds_threshold )
        {
          combo_image[idx++] = channel_image[blue][cnt];
          combo_image[idx++] = channel_image[blue][cnt];
          combo_image[idx++] = channel_image[blue][cnt];
        }
        else /* Just combine channels */
        {
          /* Reduce Red channel luminance as specified in config file */
          combo_image[idx++] = rc_data.norm_range[RED][NORM_RANGE_BLACK] +
            ( channel_image[red][cnt] * range_red ) / MAX_WHITE;
          combo_image[idx++] = channel_image[green][cnt];
          combo_image[idx++] = channel_image[blue][cnt];
        }
      } /* for( cnt = 0; cnt < (int)channel_image_size; cnt++ ) */

    } /* if( isFlagSet(IMAGE_COLORIZE) ) */
    else
    {
      /* Else combine channel images after changing pixel
       * value range to that specified in the config file */
      range_red =
        rc_data.norm_range[RED][NORM_RANGE_WHITE] -
        rc_data.norm_range[RED][NORM_RANGE_BLACK];
      range_green =
        rc_data.norm_range[GREEN][NORM_RANGE_WHITE] -
        rc_data.norm_range[GREEN][NORM_RANGE_BLACK];
      range_blue =
        rc_data.norm_range[BLUE][NORM_RANGE_WHITE] -
        rc_data.norm_range[BLUE][NORM_RANGE_BLACK];

      for( cnt = 0; cnt < channel_image_size; cnt++ )
      {
        combo_image[idx++] = rc_data.norm_range[RED][NORM_RANGE_BLACK] +
          ( channel_image[red][cnt] * range_red ) / MAX_WHITE;
        combo_image[idx++] = rc_data.norm_range[GREEN][NORM_RANGE_BLACK] +
          ( channel_image[green][cnt] * range_green ) / MAX_WHITE;
        combo_image[idx++] = rc_data.norm_range[BLUE][NORM_RANGE_BLACK] +
          ( channel_image[blue][cnt] * range_blue ) / MAX_WHITE;
      } /* for( cnt = 0; cnt < (int)channel_image_size; cnt++ ) */
    }

} /* Create_Combo_Image() */
