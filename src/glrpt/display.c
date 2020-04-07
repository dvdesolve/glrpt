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

#include "display.h"

#include "../common/shared.h"
#include "../lrpt_demod/demod.h"
#include "../sdr/ifft.h"
#include "utils.h"

#include <cairo/cairo.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*****************************************************************************/

static int IFFT_Bin_Value(int sum_i, int sum_q, gboolean reset);
static void Colorize(guchar *pix, int pixel_val);

/*****************************************************************************/

/* IFFT_Bin_Value()
 *
 * Calculates IFFT bin values with auto level control
 */
static int IFFT_Bin_Value(int sum_i, int sum_q, gboolean reset) {
  /* Value of ifft output "bin" */
  static int bin_val = 0;

  /* Maximum value of ifft bins */
  static int bin_max = 1000, max = 0;

  /* Calculate sliding window average of max bin value */
  if( reset )
  {
    bin_max = max;
    if( !bin_max ) bin_max = 1;
    max = 0;
  }
  else
  {
    /* Calculate average signal power at each frequency (bin) */
    bin_val  = bin_val * AMPL_AVE_MUL;
    bin_val += sum_i * sum_i + sum_q * sum_q;
    bin_val /= AMPL_AVE_WIN;

    /* Record max bin value */
    if( max < bin_val )
      max = bin_val;

    /* Scale bin values to 255 depending on max value */
    int ret = (255 * bin_val) / bin_max;
    if( ret > 255 ) ret = 255;
    return( ret );
  }

  return( 0 );
}

/*****************************************************************************/

/* Color codes the pixels of the
 * waterfall according to their value
 */
static void Colorize(guchar *pix, int pixel_val) {
  int n;

  if( pixel_val < 64 ) // From black to blue
  {
    pix[0] = 0;
    pix[1] = 0;
    pix[2] = 3 + (guchar)pixel_val * 4;
  }
  else if( pixel_val < 128 ) // From blue to green
  {
    n = pixel_val - 127; // -63 <= n <= 0
    pix[0] = 0;
    pix[1] = 255 + (guchar)n * 4;
    pix[2] = 3   - (guchar)n * 4;
  }
  else if( pixel_val < 192 ) // From green to yellow
  {
    n = pixel_val - 191; // -63 <= n <= 0
    pix[0] = 255 + (guchar)n * 4;
    pix[1] = 255;
    pix[2] = 0;
  }
  else // From yellow to reddish-orange
  {
    n = pixel_val - 255; // -63 <= n <= 0
    pix[0] = 255;
    pix[1] = 66 - (guchar)n * 3;
    pix[2] = 0;
  }
}

/*****************************************************************************/

/* Display_Waterfall()
 *
 * Displays IFFT Spectrum as "waterfall"
 */
void Display_Waterfall(void) {
  int
    vert_lim,  /* Limit of vertical index for copying lines */
    idh, idv,  /* Index to hor. and vert. position in warterfall */
    pixel_val, /* Greyscale value of pixel derived from ifft o/p  */
    idf,       /* Index to ifft output array */
    i, len;

  /* Pointer to current pixel */
  static guchar *pix;


  /* Copy each line of waterfall to next one */
  vert_lim = wfall_height - 2;
  for( idv = vert_lim; idv > 0; idv-- )
  {
    pix = wfall_pixels + wfall_rowstride * idv + wfall_n_channels;

    for( idh = 0; idh < wfall_width; idh++ )
    {
      pix[0] = pix[ -wfall_rowstride];
      pix[1] = pix[1-wfall_rowstride];
      pix[2] = pix[2-wfall_rowstride];
      pix += wfall_n_channels;
    }
  }

  /* Go to top left +1 hor. +1 vert. of pixbuf */
  pix = wfall_pixels + wfall_rowstride + wfall_n_channels;

  /* IFFT produces an output of positive and negative
   * frequencies and it output is handled accordingly */
  IFFT( ifft_data );

  /* Calculate bin values after IFFT */
  len = ifft_data_length / 4;

  /* Do the "positive" frequencies */
  idf = ifft_data_length / 2;
  for( i = 0; i < len; i++ )
  {
    /* Calculate vector magnitude of
     * signal at each freq. ("bin") */
    pixel_val = IFFT_Bin_Value(
        ifft_data[idf], ifft_data[idf + 1], FALSE );
    idf += 2;

    /* Color code signal strength */
    Colorize( pix, pixel_val );
    pix += wfall_n_channels;

  } /* for( i = 1; i < len; i++ ) */

  /* Do the "negative" frequencies */
  idf = 2;
  for( i = 1; i < len; i++ )
  {
    /* Calculate vector magnitude of
     * signal at each freq. ("bin") */
    pixel_val = IFFT_Bin_Value(
        ifft_data[idf], ifft_data[idf + 1], FALSE );
    idf += 2;

    /* Color code signal strength */
    Colorize( pix, pixel_val );
    pix += wfall_n_channels;

  } /* for( i = 1; i < len; i++ ) */

  /* Reset function */
  IFFT_Bin_Value( ifft_data[0], ifft_data[0], TRUE );

  /* At last draw waterfall */
  gtk_widget_queue_draw( ifft_drawingarea );

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
}

/*****************************************************************************/

/*  Display_QPSK_Const()
 *
 *  Displays the QPSK constellation
 */
void Display_QPSK_Const(int8_t *buffer) {
  /* Pointer to current pixel */
  static guchar *pix;

  /* Horizontal and Vertical index
   * to QPSK drawingarea pixels */
  gint idh, idv;

  int cnt, idx = 0;

  for( cnt = 0; cnt < QPSK_CONST_POINTS; cnt++ )
  {
    /* Position of QPSK constellation point */
    idh = qpsk_center_x +  buffer[idx++] / 2;
    idv = qpsk_center_y -  buffer[idx++] / 2;

    /* Plot the QPSK constellation point */
    pix = qpsk_pixels + qpsk_rowstride * idv + qpsk_n_channels * idh;
    pix[0] = 0xff;
    pix[1] = 0xff;
    pix[2] = 0xff;
  }

  /* Wait for GTK to complete its tasks */
  gtk_widget_queue_draw( qpsk_drawingarea );
  while( g_main_context_iteration(NULL, FALSE) );
}

/*****************************************************************************/

/* Display_Icon()
 *
 * Sets an icon to be displayed in a GTK_IMAGE
 */
void Display_Icon(GtkWidget *img, const gchar *name) {
  /* Set the icon in the image */
  gtk_image_set_from_icon_name(
      GTK_IMAGE(img), name, GTK_ICON_SIZE_BUTTON );

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
}

/*****************************************************************************/

/* Display_Demod_Params()
 *
 * Displays Demodulator parameters (AGC gain PLL freq etc)
 */
void Display_Demod_Params(Demod_t *demod) {
  char txt[10];
  double freq, gain;
  uint32_t level;

  /* Display AGC Gain and Signal Level */
  Agc_Gain( &gain );
  snprintf( txt, sizeof(txt), "%6.3f", gain );
  gtk_entry_set_text( GTK_ENTRY(agc_gain_entry), txt );
  Signal_Level( &level );
  snprintf( txt, sizeof(txt), "%6u", level );
  gtk_entry_set_text( GTK_ENTRY(sig_level_entry), txt );

  /* Display Costas PLL Frequency FIXME */
  freq = demod->costas->nco_freq * demod->sym_rate / M_2PI;
  if( (rc_data.psk_mode == DOQPSK) ||
      (rc_data.psk_mode == IDOQPSK) )
    freq *= 2.0;
  snprintf( txt, sizeof(txt), "%+8d", (int)freq );
  gtk_entry_set_text( GTK_ENTRY(pll_freq_entry), txt );

  /* Display Costas PLL Lock Detect Level */
  snprintf( txt, sizeof(txt), "%6.3f", demod->costas->moving_average );
  gtk_entry_set_text( GTK_ENTRY(pll_ave_entry), txt );

  /* Draw the level gauges */
  gtk_widget_queue_draw( sig_level_drawingarea );
  gtk_widget_queue_draw( sig_qual_drawingarea );
  gtk_widget_queue_draw( agc_gain_drawingarea );
  gtk_widget_queue_draw( pll_ave_drawingarea );

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );
}

/*****************************************************************************/

/* Draw_Level_Gauge()
 *
 * Draws a color-coded level gauge into a drawingarea
 */
void Draw_Level_Gauge(GtkWidget *widget, cairo_t *cr, double level) {
  GtkAllocation allocation;
  double dWidth, dHeight;
  int iWidth, idx;
  double red, green;

  /* Get size of drawingarea widget */
  gtk_widget_get_allocation( widget, &allocation );

  /* The -1 puts the margin rectangle in the drawingarea */
  dWidth  = (double)( allocation.width  - 1 );
  dHeight = (double)( allocation.height - 1 );

  /* Width to which gauge to be drawn, depending
   * on level. The -3 is needed to leave a margin
   * of 1 pixel around the colored bar */
  iWidth = (int)( (double)(allocation.width - 3) * level );

  /* Set line width to 1 pixel */
  cairo_set_line_width( cr, 1.0 );

  /* Draw a black outline around drawingarea */
  cairo_rectangle( cr, 0.5, 0.5, dWidth, dHeight );
  cairo_set_source_rgb( cr, 0.4, 0.4, 0.4 );
  cairo_stroke( cr );

  /* Fill the drawingarea grey, margin = 1.0 line */
  dHeight--;
  dWidth--;
  cairo_rectangle( cr, 1.0, 1.0, dWidth, dHeight );
  cairo_set_source_rgb( cr, 0.87, 0.87, 0.87 );
  cairo_fill( cr );

  /* These calculations arrange for the value of red and green
   * to be 1.0 (full) over a range of values of "level" */
  level /= TRANSITION_BAND;
  red = RED_THRESHOLD - level;
  red = dClamp( red, 0.0, 1.0 );
  green = level - GREEN_THRESHOLD;
  green = dClamp( green, 0.0, 1.0 );

  /* Draw vertical lines to width depending on level supplied */
  for( idx = 1; idx < iWidth; idx++ )
  {
    double dIdx = (double)idx;
    cairo_move_to( cr, dIdx + 1.5, 2.0 );
    cairo_line_to( cr, dIdx + 1.5, dHeight );
    cairo_set_source_rgb( cr, red, green, 0.0 );
    cairo_stroke( cr );
  }
}
