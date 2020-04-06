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

#ifndef GLRPT_INTERFACE_H
#define GLRPT_INTERFACE_H

#include <gtk/gtk.h>

#include <stddef.h>

/* Gtk Builder object ids */
#define ERROR_DIALOG_IDS \
  "error_dialog", \
  "error_ok_button", \
  "error_quit_button", \
  NULL

#define POPUP_MENU_IDS \
  "popup_menu", \
  "start_receiver_menuitem", \
  "decode_images_menuitem", \
  "decode_timer_menuitem", \
  "auto_timer_menuitem", \
  "cancel_timer_menuitem", \
  "select_satellite", \
  "select_satellite_menu", \
  "raw_menuitem", \
  "normalize_menuitem", \
  "clahe_menuitem", \
  "rectify_menuitem", \
  "invert_menuitem", \
  "combine_menuitem", \
  "individual_menuitem", \
  "pseudo_menuitem", \
  "jpeg_menuitem", \
  "pgm_menuitem", \
  "quit_menuitem", \
  NULL

#define MAIN_WINDOW_IDS \
  "main_window", \
  "image_scrolledwindow", \
  "image_viewport", \
  "lrpt_image", \
  "satellite_label", \
  "text_scrolledwindow", \
  "message_textview", \
  "qpsk_drawingarea", \
  "ifft_drawingarea", \
  "sdr_freq_entry", \
  "sdr_bw_entry", \
  "sdr_tuner_entry", \
  "sdr_gain_scale", \
  "sdr_gain_adjust", \
  "start_togglebutton", \
  "manual_gain_radiobutton", \
  "auto_gain_radiobutton", \
  "pll_ave_entry", \
  "pll_freq_entry", \
  "pll_lock_icon", \
  "frame_icon", \
  "status_icon", \
  "sig_level_entry", \
  "sig_quality_entry", \
  "agc_gain_entry", \
  "packet_cnt_entry", \
  "ob_time_entry", \
  "sig_level_drawingarea", \
  "on_sig_qual_drawingarea", \
  "agc_gain_drawingarea", \
  "on_pll_ave_drawingarea", \
  NULL

#define QUIT_DIALOG_IDS \
  "quit_dialog", \
  "quit_cancel_button", \
  NULL

#define AUTO_TIMER_DIALOG_IDS \
  "auto_timer_dialog", \
  "auto_timer_cancel_button", \
  "auto_timer_ok_button", \
  NULL

#define OPER_TIMER_DIALOG_IDS \
  "decode_timer_dialog", \
  "decode_timer_ok_button", \
  "decode_timer_adj", \
  "decode_timer_spinbutton", \
  NULL

GtkWidget *create_main_window(GtkBuilder **builder);
GtkWidget *create_error_dialog(GtkBuilder **builder);
GtkWidget *Builder_Get_Object(GtkBuilder *builder, gchar *name);
GtkWidget *create_popup_menu(GtkBuilder **builder);
GtkWidget *create_timer_dialog(GtkBuilder **builder);
GtkWidget *create_startstop_timer(GtkBuilder **builder);
GtkWidget *create_quit_dialog(GtkBuilder **builder);
void Initialize_Top_Window(void);

#endif
