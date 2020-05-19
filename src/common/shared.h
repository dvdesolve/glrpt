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

#ifndef COMMON_SHARED_H
#define COMMON_SHARED_H

/*****************************************************************************/

#include "../decoder/huffman.h"
#include "../decoder/met_to_data.h"
#include "../glrpt/rc_config.h"
#include "../sdr/filters.h"
#include "common.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <semaphore.h>
#include <stddef.h>
#include <stdint.h>

/*****************************************************************************/

/* Runtime config data */
extern rc_data_t rc_data;

/* QPSK constellation drawing area pixbuf */
extern GdkPixbuf *qpsk_pixbuf;
extern guchar    *qpsk_pixels;
extern gint
    qpsk_rowstride,
    qpsk_n_channels,
    qpsk_width,
    qpsk_height,
    qpsk_center_x,
    qpsk_center_y;

/* Waterfall window pixbuf */
extern GdkPixbuf *wfall_pixbuf;
extern guchar    *wfall_pixels;
extern gint
    wfall_rowstride,
    wfall_n_channels,
    wfall_width,
    wfall_height;

/* Global widgets */
extern GtkWidget
    *qpsk_drawingarea,    /* QPSK constellation drawing area                  */
    *ifft_drawingarea,    /* IFFT spectrum drawing area                       */
    *main_window,         /* glrpt's top window                               */
    *start_togglebutton,  /* Start receive and decode toggle button           */
    *text_scroller,       /* Text view scroller                               */
    *lrpt_image,          /* Image to be displayed                            */
    *pll_lock_icon,       /* PLL lock indicator icon                          */
    *pll_ave_entry,       /* PLL lock detect level                            */
    *pll_freq_entry,      /* PLL frequency indicator                          */
    *sig_level_entry,     /* Average signal level in AGC                      */
    *agc_gain_entry,      /* AGC gain level                                   */
    *frame_icon,          /* Frame status indicator icon                      */
    *status_icon,         /* Receiver status indicator icon                   */
    *sig_quality_entry,   /* Signal quality as given by packet decoder        */
    *packet_cnt_entry,    /* OK and total count of packets                    */
    *ob_time_entry,       /* Onboard time indicator                           */
    *sig_level_drawingarea, /* Signal level drawing area                      */
    *sig_qual_drawingarea,  /* Signal quality drawing area                    */
    *agc_gain_drawingarea,  /* AGC gain drawing area                          */
    *pll_ave_drawingarea;   /* PLL average drawing area                       */

extern GtkBuilder
    *decode_timer_dialog_builder,
    *auto_timer_dialog_builder,
    *main_window_builder,
    *popup_menu_builder;

/* Text buffer for text view */
extern GtkTextBuffer *text_buffer;

/* Pixbuf for scaled images display */
extern GdkPixbuf *scaled_image_pixbuf;

/* Pixbuf rowstride and num of channels */
extern gint
  scaled_image_width,
  scaled_image_height,
  scaled_image_rowstride,
  scaled_image_n_channels;

/* Pixel buffer for scaled images display */
extern guchar *scaled_image_pixel_buf;

/* Common between callbacks.c and callback_func.c */
extern GtkWidget
    *quit_dialog,
    *error_dialog,
    *popup_menu,
    *decode_timer_dialog,
    *auto_timer_dialog;

/* IFFT data buffer */
extern int16_t *ifft_data;
extern uint16_t ifft_data_length;

/* Chebyshev filter data I/Q */
extern filter_data_t filter_data_i;
extern filter_data_t filter_data_q;

/* Demodulator control semaphore */
extern sem_t demod_semaphore;

/* Meteor decoder variables */
extern ac_table_rec_t *ac_table;
extern size_t ac_table_len;
extern mtd_rec_t mtd_record;

/* Channel images and sizes */
extern uint8_t *channel_image[CHANNEL_IMAGE_NUM];
extern size_t   channel_image_size;
extern uint32_t channel_image_width, channel_image_height;

/*****************************************************************************/

#endif
