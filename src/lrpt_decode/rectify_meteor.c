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

/*
 * These functions have been derived from rectify-jpg.c kindly
 * provided by Rich Griffiths W2RG
 *
 * rectify-jpg.c 23 May 2019    by RAGS
 *                  revised     24 May 2019
 *                  revised      2 June 2019
 *                  revised      4 June 2019
 *                  revised      6 June 2019
 *                  revised     10 June 2019
 *                  revised     07 July 2019
 */

/* Quote from original rectify-jpg.c:
 *
 * The pixels are uniformly spaced, which is an error referred to as tangential
 * scale distortion. An additional source of distortion is due to the curvature
 * of Earth. rectify-jpg calculates what the correct spacing of the pixels should
 * be based on how the satellite's scanner operates and the geometry of the orbit.
 * rectify-jpg then constructs a new line with properly spaced RGB pixels, using
 * interpolated pixel values to fill the gaps.
 */

/*****************************************************************************/

#include "rectify_meteor.h"

#include "../common/shared.h"
#include "../glrpt/utils.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/

static double Calculate_beta(double phi);
static void Rectify_Grayscale_1(
        uint8_t *in_buff,
        uint32_t in_width,
        uint32_t in_height,
        uint8_t *rect_buff);
static void Calculate_Pixel_Spacing_1(uint32_t in_width, uint32_t *rect_width);
static void Rectify_Grayscale_2(
        uint8_t *in_buff,
        uint32_t in_width,
        uint32_t in_height,
        uint8_t *rect_buff);
static void Calculate_Pixel_Spacing_2(
        uint32_t orig_width,
        uint32_t *rect_width);

/*****************************************************************************/

/* Gap between rectified pixels */
static uint8_t *gap = NULL;

/* Array that holds buffer indices for the reference
 * pixels that are the right ones to use to extrapolate
 * the value of rectified image pixels */
static uint32_t *indices = NULL;

/* Array of extrapolation factors needed to
 * calculate rectified image's pixel values */
static double *factors = NULL;

/*****************************************************************************/

/* Calculate_beta()
 *
 * Calculates beta, the angle between the scanner's "contact"
 * point on the surface of Earth and the sub-satellite point.
 * This angle is from the Earth's center to these points. This
 * function uses the Newton-Raphson method to numerically solve
 * the equation R*sin(beta)-[A + R(1-cos(beta))]*tan(phi) = 0.
 * A = Satellite Altitude R = Earth Radius phi = Scanner angle */
static double Calculate_beta(double phi) {
  double
    beta_np1 = 0.0,  /* New beta value, beta_n+1 */
             sin_b, cos_b,    /* sin and cos of current beta_n */
             f_beta, df_beta; /* The function of beta and derivative */

  static double beta_n = 0.1; /* Starting value of beta_n */

  double
    tan_phi, /* tan(phi), the scanner's angle */
    aRp1,    /* (1 + A/R) * tan(phi) */
    delta;   /* The convergence criterion */

  tan_phi = tan( phi );
  aRp1    = ( 1.0 + SAT_ALTITUDE / EARTH_RADIUS ) * tan_phi;
  delta   = 1.0;

  /* Loop over Newton-Raphson iteration */
  while( fabs(delta) > 0.00001 )
  {
    sin_b    = sin( beta_n );
    cos_b    = cos( beta_n );
    f_beta   = sin_b + cos_b * tan_phi - aRp1;
    df_beta  = cos_b - sin_b * tan_phi;

    /* New improved estimate of beta */
    beta_np1 = beta_n - f_beta / df_beta;
    delta    = ( beta_np1 - beta_n ) / beta_np1;
    beta_n   = beta_np1;
  }

  return( beta_np1 );
}

/*****************************************************************************/

/* Rectify_Grayscale_1()
 *
 * Corrects tangential geometric distortion and the effect
 * of Earth's curvature on the raw Meteor-M images.
 */
static void Rectify_Grayscale_1(
        uint8_t *in_buff,
        uint32_t in_width,
        uint32_t in_height,
        uint8_t *rect_buff) {
  uint8_t
    byteA_right = 0,
    byteB_right = 0,
    byteA_left  = 0,
    byteB_left  = 0;

  uint32_t
    in_width2,
    ch_width2,
    line_count,
    in_buff_right,
    in_buff_left,
    rect_buff_right,
    rect_buff_left,
    idx;


  /* Rectify image buffer line by line */
  in_width2 = in_width / 2;
  ch_width2 = channel_image_width / 2;
  for( line_count = 0; line_count < in_height; line_count++ )
  {
    /* Middle of each line in the rectified image */
    rect_buff_right = line_count * channel_image_width + ch_width2;
    rect_buff_left  = rect_buff_right - 1;

    /* Middle of each line in the input image */
    in_buff_right = line_count * in_width + in_width2;
    in_buff_left  = in_buff_right - 1;

    /* Get middle pixel right from input */
    byteA_right = *( in_buff + in_buff_right++ );

    /* Put it on the rectified line */
    *( rect_buff + rect_buff_right++ ) = byteA_right;

    /* Get middle pixel left from input */
    byteA_left = *( in_buff + in_buff_left-- );

    /* Put it on the rectified line */
    *( rect_buff + rect_buff_left-- ) = byteA_left;

    /* Traverse the rest of the line */
    for( idx = 0; idx < in_width2 - 1; idx++ )
    {
      /* Get the next pixel */
      byteB_right = *( in_buff + in_buff_right++ );
      byteB_left  = *( in_buff + in_buff_left-- );

      /* Fill the gap between the two pixels */
      switch ( *(gap + idx) )
      {
        case 0:
          break;

          /* Pad the space */
        case 1:
          *( rect_buff + rect_buff_right++ ) = ( byteA_right + byteB_right ) / 2;
          *( rect_buff + rect_buff_left-- )  = ( byteA_left  + byteB_left )  / 2;
          break;

        case 2:
          *( rect_buff + rect_buff_right++ ) = ( 2 * byteA_right + byteB_right ) / 3;
          *( rect_buff + rect_buff_right++ ) = ( byteA_right + 2 * byteB_right ) / 3;
          *( rect_buff + rect_buff_left-- )  = ( 2 * byteA_left  + byteB_left )  / 3;
          *( rect_buff + rect_buff_left-- )  = ( byteA_left + 2  * byteB_left )  / 3;
          break;

        case 3:
          *( rect_buff + rect_buff_right++ ) = ( 2 * byteA_right + byteB_right ) / 3;
          *( rect_buff + rect_buff_right++ ) = ( byteA_right     + byteB_right ) / 2;
          *( rect_buff + rect_buff_right++ ) = ( byteA_right     + 2 * byteB_right ) / 3;
          *( rect_buff + rect_buff_left-- )  = ( 2 * byteA_left  + byteB_left ) / 3;
          *( rect_buff + rect_buff_left-- )  = ( byteA_left      + byteB_left ) / 2;
          *( rect_buff + rect_buff_left-- )  = ( byteA_left      + 2 * byteB_left ) / 3;
          break;

        case 4:
          *( rect_buff + rect_buff_right++ ) = ( 3 * byteA_right + byteB_right ) / 4;
          *( rect_buff + rect_buff_right++ ) = ( 2 * byteA_right + byteB_right ) / 3;
          *( rect_buff + rect_buff_right++ ) = ( byteA_right + 2 * byteB_right ) / 3;
          *( rect_buff + rect_buff_right++ ) = ( byteA_right + 3 * byteB_right ) / 4;
          *( rect_buff + rect_buff_left-- )  = ( 3 * byteA_left  + byteB_left )  / 4;
          *( rect_buff + rect_buff_left-- )  = ( 2 * byteA_left  + byteB_left )  / 3;
          *( rect_buff + rect_buff_left-- )  = ( byteA_left + 2  * byteB_left )  / 3;
          *( rect_buff + rect_buff_left-- )  = ( byteA_left + 3  * byteB_left )  / 4;
          break;

        default:
          fprintf(stderr, "\nglrpt: A problem occurred spacing the pixels\n" );
          break;
      } /* switch ( *( gap + idx) ) */

      /* And write the pixel */
      *( rect_buff + rect_buff_right++ ) = byteB_right;
      byteA_right  = byteB_right;
      *( rect_buff + rect_buff_left-- )  = byteB_left;
      byteA_left   = byteB_left;

    } /* for( idx = 0; idx < in_width-1; idx++ ) */
  } /* for( line_count = 0; line_count < height; line_count++ ) */
}

/*****************************************************************************/

/* Calculate_Pixel_Spacing_1()
 *
 * Calculates the correct pixel spacing of Meteor-M images taking into
 * account the Earth's curvature and the scanner's tangential distortion
 */
static void Calculate_Pixel_Spacing_1(uint32_t in_width, uint32_t *rect_width) {
  /* A little geometry of the satellite, Earth, and the scans */
  double
    phi,         // instantaneous scan angle from vertical
    delta_phi,   // incremental scan angle pixel-to-pixel
    beta,        // Angle on center of earth corresponding to phi
    beta_max,
    resolution,
    dwidth;

  uint32_t
    idx,
    in_width2; // Middle right pixel of input image

  /* Unfilled gaps between true pixel locations */
  double unusedspace = 0.0;

  /* New position of rectified pixels */
  in_width2 = in_width / 2;
  double *newposition = NULL;
  if( newposition == NULL )
    mem_alloc( (void **) &newposition, (size_t)in_width2 * sizeof(double) );

  /* Gap between pixels */
  if( gap == NULL )
    mem_alloc( (void **)&gap, (size_t)(in_width2 - 1) * sizeof(int) );

  /* Stride pixel-to-pixel of the scanner, in rad */
  delta_phi = 2.0 * PHI_MAX / (double)( in_width - 1 );

  /* Max beta angle, corresponding to Max phi  */
  beta_max = Calculate_beta( PHI_MAX );

  /* The size of first sub-satellite pixel */
  resolution = 2.0 * Calculate_beta( delta_phi / 2.0 );

  /* This is the width of rectified image in float */
  dwidth = 2.0 * beta_max / resolution;

  /* And this is the width in pixels of the rectified image */
  *rect_width = (uint32_t)dwidth;

  /* Now rounded to nearest multiple of 8 because
   * this is prefered by the built-in JPEG compressor */
  *rect_width = ( *rect_width / 8 ) * 8;

  /* Reference size of pixels in rectified image */
  resolution = 2.0 * beta_max / (double)( *rect_width - 1 );

  /* Calculate the correct position of each pixel */
  /* Calculate the correct position of each pixel */
  for( idx = 0; idx < in_width2; idx++ )
  {
    phi  = ( (double)idx + 0.5 ) * delta_phi;
    beta = Calculate_beta( phi );
    newposition[idx] = beta / resolution;
  }

  /* Calculate number of gaps between rectified pixels */
  in_width2--;
  for( idx = 0; idx < in_width2; idx++ )
  {
    unusedspace += newposition[ idx + 1 ] - newposition[ idx ] - 1.0;
    if( unusedspace >= 4.0 )
    {
      gap[ idx ]   = 4;
      unusedspace -= 4.0;
    }
    else if( unusedspace >= 3.0 )
    {
      gap[ idx ]   = 3;
      unusedspace -= 3.0;
    }
    else if( unusedspace >= 2.0 )
    {
      gap[ idx ]   = 2;
      unusedspace -= 2.0;
    }
    else if( unusedspace >= 1.0 )
    {
      gap[ idx ]   = 1;
      unusedspace -= 1.0;
    }
    else
    {
      gap[ idx ] = 0;
    }
  }

  free_ptr( (void **) &newposition );
}

/*****************************************************************************/

/* Rectify_Grayscale_2()
 *
 * Corrects tangential geometric distortion and the effect
 * of Earth's curvature on the raw Meteor-M scanner images.
 */
static void Rectify_Grayscale_2(
        uint8_t *in_buff,
        uint32_t in_width,
        uint32_t in_height,
        uint8_t *rect_buff) {
  uint32_t
    in_buff_idx,    /* Index to the unrectified input image buffer    */
    rect_buff_idx,  /* Index to the rectified output image buffer     */
    in_width2,      /* Half the width (center) of input image buffer  */
    rect_width2;    /* Half the width (center) of output image buffer */

  uint32_t
    horiz_cnt,  /* Count of pixels created in the rectified image buffer */
    vert_cnt,   /* Count of lines of input/output images processed       */
    in_stride;  /* Dist. in pixels of the current image line into buffer */

  double
    pixel_value_diff,   /* Difference of values of two adjacent input pixels */
    pixel_value;        /* Extrapolated value of a pixel in rectified buffer */

  /* The center pixels (first to right of center) of images */
  in_width2   = in_width / 2;
  rect_width2 = channel_image_width / 2;

  /* Rectify images lane by line */
  for( vert_cnt = 0; vert_cnt < in_height; vert_cnt++ )
  {
    /* Indices to input and output image buffers */
    rect_buff_idx = rect_width2 + vert_cnt * channel_image_width;
    in_stride     = in_width2   + vert_cnt * in_width;

    /* Extrapolate values of each pixel in output buffer.
     * This is from center right pixel to the right edges */
    for( horiz_cnt = 0; horiz_cnt < rect_width2; horiz_cnt++ )
    {
      /* This index points to pixel in input buffer that is to
       * be used as the reference for extrapolating pixel value */
      in_buff_idx = indices[horiz_cnt] + in_stride;

      /* This is the diff in values of the reference pixel and the one
       * before it, and it is to be used for linear extrapolation of the
       * value of the current pixel of the output (rectified) buffer */
      pixel_value_diff = in_buff[in_buff_idx - 1] - in_buff[in_buff_idx];

      /* Pixel value is the reference input pixel value  plus a propotion
       * of the value diff above, according to the extrapolation factor */
      pixel_value = in_buff[in_buff_idx] + pixel_value_diff * factors[horiz_cnt];
      rect_buff[rect_buff_idx] = (uint8_t)pixel_value;
      rect_buff_idx++;
    }

    /* Extrapolate values of each pixel in output buffer as above.
     * This is from the center left pixel to the left edges */
    rect_buff_idx = rect_width2 + vert_cnt * channel_image_width;
    for( horiz_cnt = 0; horiz_cnt < rect_width2; horiz_cnt++ )
    {
      in_buff_idx = in_stride - indices[horiz_cnt];
      pixel_value_diff = in_buff[in_buff_idx - 1] - in_buff[in_buff_idx];
      pixel_value = in_buff[in_buff_idx - 1] - pixel_value_diff * factors[horiz_cnt];
      rect_buff_idx--;
      rect_buff[rect_buff_idx] = (uint8_t)pixel_value;
    }
  } /* for( horiz_cnt = 0; horiz_cnt < rect_width2; horiz_cnt++ ) */
}

/*****************************************************************************/

/* Calculate_Pixel_Spacing_2()
 *
 * Calculates the correct pixel spacing of Meteor-M
 * images taking into account the Earth's curvature
 * and the scanner's tangential distortion
 */
static void Calculate_Pixel_Spacing_2(
        uint32_t orig_width,
        uint32_t *rect_width) {
  double
    beta_max,   /* Max beta angle, corresponding to PHI_MAX (54 deg) */
    phi,        /* Current scanner angle */
    delta_phi,  /* Scanner angle delta from pixel to pixel of orig. image  */
    delta_phi2, /* Half the above delta phi */
    beta,       /* The current beta angle corresponding to current phi */
    delta_center,   /* The center to center distance of rectified image pixels */
    delta_center2,  /* Half the above delta */
    orig_pixel_center, /* Distance of orig. pixels' center from sub-satellite point */
    rect_pixel_center, /* Distance of rect. pixels' center from sub-satellite point */
    prev_center,       /* Distance as above of the previous pixel */
    dwidth;

  uint32_t
    rect_idx,    /* Index to rectified image buffer */
    orig_idx,    /* Index to original image buffer  */
    rect_center; /* Center pixel of rectified image line */

  size_t req;

  /* Calculate beta_max and the width in pixels of rectified image */
  beta_max = Calculate_beta( PHI_MAX );

  /* The angular step value of the scanner's angle phi */
  delta_phi  = 2.0 * PHI_MAX / (double)( orig_width - 1 );
  delta_phi2 = delta_phi / 2.0;

  /* The rectified image's pixels are along the arc on the
   * surface of Earth, and the original scanner image's pixels
   * are effectively on a circle of radius SAT_ALTITUDE. This
   * is the ratio of rectified image to original image width */
  dwidth = beta_max / Calculate_beta( delta_phi2 );

  /* And this is the width in pixels of the rectified image */
  *rect_width = (uint32_t)( ceil(dwidth) );

  /* Now rounded to nearest multiple of 8 because
   * this is prefered by the built-in JPEG compressor */
  *rect_width = ( *rect_width / 8 ) * 8;

  /* Allocate the indices buffer. It holds indices to the buffer
   * of the original image for the appropriate pixels to use
   * to extrapolate pixels values of the rectified image */
  req = (size_t)*rect_width * sizeof(uint32_t) / 2;
  mem_alloc( (void **)&indices, req );

  /* Allocate the extrapolation factors buffer. It holds
   * the appropriate extrapolation factors to calculte
   * pixel values of the rectified image */
  req = (size_t)*rect_width * sizeof(double) / 2;
  mem_alloc( (void **)&factors, req );

  /* Center pixel (first to right of sub-satellite point) of rectified image */
  rect_center = *rect_width / 2;

  /* Distance between centers of adjacent rectified image pixels */
  delta_center  = 2.0 * beta_max / (double)( *rect_width - 1 );
  delta_center2 = delta_center / 2.0;

  orig_idx = 0;
  rect_idx = 0;
  prev_center = - Calculate_beta( delta_phi2 );
  /* Repeat for all pixels in rectified image buffer */
  while( rect_idx < rect_center )
  {
    /* The current value of scanner angle phi */
    phi = (double)orig_idx * delta_phi + delta_phi2;

    /* The current value of beta */
    beta = Calculate_beta( phi );

    /* The center's position of original image pixels
     * when projected onto the surface of the Earth */
    orig_pixel_center = beta;

    /* The center's position of the rectified image pixels
     * when projected onto the surface of the Earth */
    rect_pixel_center = delta_center * (double)rect_idx + delta_center2;

    /* If rectified pixel overtakes an original pixel, then
     * advance the original pixel's index and save its center
     * position as the previous center */
    if( rect_pixel_center > orig_pixel_center )
    {
      orig_idx++;
      prev_center = orig_pixel_center;
      continue;
    }

    /* If rectified pixel's center position is less than
     * the original pixel's position, save the original
     * pixel's index in the reference indices buffer */
    indices[rect_idx] = orig_idx;

    /* The extrapolation factor is the distance of the rectified
     * pixel's center from the reference pixel's center, divided
     * by the distance between the centers of the reference pixel
     * and the previous one */
    factors[rect_idx]  = rect_pixel_center - orig_pixel_center;
    factors[rect_idx] /= prev_center - orig_pixel_center;
    rect_idx++;
  } /* while( rect_idx < rect_center ) */
}

/*****************************************************************************/

/* Rectify_Images()
 *
 * Rectifies (corrects geometric distortion) of Meteor images
 */
void Rectify_Images(void) {
  /* Create a temp image buffer to save original images
   * and re-allocate channel images to be rectified */
  uint8_t *temp_image = NULL;
  size_t   orig_size  = (size_t)( channel_image_width * channel_image_height );
  orig_size *= sizeof(uint8_t);
  mem_alloc( (void **) &temp_image, orig_size );

  /* Initialize rectifying functions. channel_image_width
   * will become the new width of the rectified images */
  switch( rc_data.rectify_function )
  {
    case 1:
      Show_Message( "Using Rectify Function 1 (W2RG)", "green" );
      Calculate_Pixel_Spacing_1( METEOR_IMAGE_WIDTH, &channel_image_width );
      break;

    case 2:
      Show_Message( "Using Rectify Function 2 (5B4AZ)", "green" );
      Calculate_Pixel_Spacing_2( METEOR_IMAGE_WIDTH, &channel_image_width );
      break;
  }

  size_t new_size = (size_t)(channel_image_width * channel_image_height);
  new_size *= sizeof(uint8_t);

  /* The size of the channel images will also increase */
  channel_image_size = new_size;

  /* Rectify image channels */
  for( uint8_t idx = 0; idx < CHANNEL_IMAGE_NUM; idx++ )
  {
    /* Save original image to temp */
    memmove( temp_image, channel_image[idx], orig_size );

    /* Re-allocate image buffer and rectify */
    mem_realloc( (void **) &channel_image[idx], new_size );
    switch( rc_data.rectify_function )
    {
      case 1:
        Rectify_Grayscale_1(
            temp_image, METEOR_IMAGE_WIDTH,
            channel_image_height, channel_image[idx] );
        break;

      case 2:
        Rectify_Grayscale_2(
            temp_image, METEOR_IMAGE_WIDTH,
            channel_image_height, channel_image[idx] );
        break;
    }
  }

  SetFlag( IMAGES_RECTIFIED );
  free_ptr( (void **) &temp_image );
}
