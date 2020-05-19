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

#include "callbacks.h"

#include "../common/shared.h"
#include "../decoder/medet.h"
#include "../decoder/met_jpg.h"
#include "../demodulator/demod.h"
#include "../sdr/SoapySDR.h"
#include "callback_func.h"
#include "display.h"
#include "interface.h"
#include "utils.h"

#include <cairo.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/*****************************************************************************/

void on_main_window_destroy(GObject *object, gpointer data) {
  gtk_main_quit();
}

/*****************************************************************************/

gboolean on_main_window_delete_event(
        GtkWidget *widget,
        GdkEvent *event,
        gpointer data) {
  GtkBuilder *builder;
  if( isFlagSet(STATUS_RECEIVING) && !quit_dialog )
  {
    quit_dialog = create_quit_dialog( &builder );
    gtk_widget_show( quit_dialog );
    g_object_unref( builder );
  }
  else gtk_main_quit();

  return TRUE;
}

/*****************************************************************************/

gboolean on_image_viewport_button_press_event(
        GtkWidget *widget,
        GdkEventButton *event,
        gpointer data) {
  if( event->button == 3 ) Popup_Menu();
  return TRUE;
}

/*****************************************************************************/

void on_decode_timer_dialog_destroy(GObject *object, gpointer data) {
  decode_timer_dialog = NULL;
}

/*****************************************************************************/

void on_decode_timer_ok_button_clicked(GtkButton *button, gpointer data) {
  Decode_Timer_Setup();
}

/*****************************************************************************/

void on_auto_timer_cancel_button_clicked(GtkButton *button, gpointer data) {
  gtk_widget_destroy( auto_timer_dialog );
}

/*****************************************************************************/

void on_auto_timer_ok_button_clicked(GtkButton *button, gpointer data) {
  Auto_Timer_OK_Clicked();
}

/*****************************************************************************/

void on_auto_timer_hrs_changed(GtkEditable *editable, gpointer data) {
  Hours_Entry( editable );
}

/*****************************************************************************/

void on_auto_timer_min_changed(GtkEditable *editable, gpointer data) {
  Minutes_Entry( editable );
}

/*****************************************************************************/

void on_auto_times_activate(GtkEntry *entry, gpointer data) {
  gtk_widget_grab_focus( GTK_WIDGET(data) );
}

/*****************************************************************************/

void on_auto_stop_min_activate(GtkEntry *entry, gpointer data) {
  Auto_Timer_OK_Clicked();
}

/*****************************************************************************/

void on_auto_timer_destroy(GObject *object, gpointer data) {
  auto_timer_dialog = NULL;
}

/*****************************************************************************/

void on_start_receiver_menuitem_toggled(
        GtkCheckMenuItem *menuitem,
        gpointer data) {
  Start_Receiver_Menuitem_Toggled( menuitem );
}

/*****************************************************************************/

void on_decode_images_menuitem_toggled(
        GtkCheckMenuItem *menuitem,
        gpointer data) {
  Decode_Images_Menuitem_Toggled( menuitem );
}

/*****************************************************************************/

void on_save_images_menuitem_activate(GtkMenuItem *menuitem, gpointer data) {
  Mj_Dump_Image();
}

/*****************************************************************************/

void on_decode_timer_menuitem_activate(GtkMenuItem *menuitem, gpointer data) {
  decode_timer_dialog =
    create_timer_dialog( &decode_timer_dialog_builder );
  gtk_widget_show( decode_timer_dialog );
}

/*****************************************************************************/

void on_auto_timer_menuitem_activate(GtkMenuItem *menuitem, gpointer data) {
  auto_timer_dialog =
    create_startstop_timer( &auto_timer_dialog_builder );
  gtk_widget_show( auto_timer_dialog );
}

/*****************************************************************************/

void on_cancel_timer_menuitem_activate(GtkMenuItem *menuitem, gpointer data) {
  g_idle_add( Cancel_Timer, NULL );
}

/*****************************************************************************/

void on_satellite_menuitem_activate(GtkMenuItem *menuitem, gpointer data) {
  /* Get menu item label */
  gchar *label = (gchar *)gtk_menu_item_get_label( menuitem );

  /* Copy satellite name to rc_data */
  Strlcpy( rc_data.satellite_name, label, sizeof(rc_data.satellite_name) );

  /* Init and enter center freq to relevant entry widget */
  g_idle_add(G_SOURCE_FUNC(Load_Config), NULL );
}

/*****************************************************************************/

void on_raw_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer data) {
  if( gtk_check_menu_item_get_active(menuitem) )
    SetFlag( IMAGE_RAW );
  else
    ClearFlag( IMAGE_RAW );
}

/*****************************************************************************/

void on_normalize_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer data) {
  if( gtk_check_menu_item_get_active(menuitem) )
    SetFlag( IMAGE_NORMALIZE );
  else
    ClearFlag( IMAGE_NORMALIZE );
}

/*****************************************************************************/

void on_clahe_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer data) {
  if( gtk_check_menu_item_get_active(menuitem) )
    SetFlag( IMAGE_CLAHE );
  else
    ClearFlag( IMAGE_CLAHE );
}

/*****************************************************************************/

void on_rectify_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer data) {
  if( gtk_check_menu_item_get_active(menuitem) )
    SetFlag( IMAGE_RECTIFY );
  else
    ClearFlag( IMAGE_RECTIFY );
}

/*****************************************************************************/

void on_invert_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer data) {
  if( gtk_check_menu_item_get_active(menuitem) )
    SetFlag( IMAGE_INVERT );
  else
    ClearFlag( IMAGE_INVERT );
}

/*****************************************************************************/

void on_combine_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer data) {
  if( gtk_check_menu_item_get_active(menuitem) )
    SetFlag( IMAGE_OUT_COMBO );
  else
    ClearFlag( IMAGE_OUT_COMBO );
}

/*****************************************************************************/

void on_individual_menuitem_toggled(
        GtkCheckMenuItem *menuitem,
        gpointer data) {
  if( gtk_check_menu_item_get_active(menuitem) )
    SetFlag( IMAGE_OUT_SPLIT );
  else
    ClearFlag( IMAGE_OUT_SPLIT );
}

/*****************************************************************************/

void on_pseudo_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer data) {
  if( gtk_check_menu_item_get_active(menuitem) )
    SetFlag( IMAGE_COLORIZE );
  else
    ClearFlag( IMAGE_COLORIZE );
}

/*****************************************************************************/

void on_jpeg_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer data) {
  if( gtk_check_menu_item_get_active(menuitem) )
    SetFlag( IMAGE_SAVE_JPEG );
  else
    ClearFlag( IMAGE_SAVE_JPEG );
}

/*****************************************************************************/

void on_pgm_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer data) {
  if( gtk_check_menu_item_get_active(menuitem) )
    SetFlag( IMAGE_SAVE_PPGM );
  else
    ClearFlag( IMAGE_SAVE_PPGM );
}

/*****************************************************************************/

void on_quit_menuitem_activate(GtkMenuItem *menuitem, gpointer data) {
  GtkBuilder *builder;
  if( isFlagSet(STATUS_RECEIVING) && !quit_dialog )
  {
    quit_dialog = create_quit_dialog( &builder );
    gtk_widget_show( quit_dialog );
    g_object_unref( builder );
  }
  else
  {
    gtk_main_quit();
  }
}

/*****************************************************************************/

gboolean on_error_dialog_delete_event(
        GtkWidget *widget,
        GdkEvent *event,
        gpointer data) {
  return TRUE;
}

/*****************************************************************************/

void on_quit_dialog_destroy(GObject *object, gpointer data) {
  quit_dialog = NULL;
}

/*****************************************************************************/

void on_error_dialog_destroy(GObject *object, gpointer data) {
  error_dialog = NULL;
}

/*****************************************************************************/

void on_error_ok_button_clicked(GtkButton *button, gpointer data) {
  gtk_widget_destroy( error_dialog );
}

/*****************************************************************************/

void on_error_quit_button_clicked(GtkButton *button, gpointer data) {
  ClearFlag( STATUS_RECEIVING );
  gtk_widget_destroy( error_dialog );
  gtk_main_quit();
}

/*****************************************************************************/

void on_quit_cancel_button_clicked(GtkButton *button, gpointer data) {
  gtk_widget_destroy( quit_dialog );
}

/*****************************************************************************/

void on_quit_button_clicked(GtkButton *button, gpointer data) {
  ClearFlag( STATUS_RECEIVING );
  gtk_main_quit();
}

/*****************************************************************************/

gboolean on_ifft_drawingarea_draw(
        GtkWidget *widget,
        cairo_t *cr,
        gpointer data) {
  if( wfall_pixbuf != NULL )
  {
    /* Draw the waterfall */
    gdk_cairo_set_source_pixbuf( cr, wfall_pixbuf, 0.0, 0.0 );
    cairo_paint( cr );
    return( TRUE );
  }
  return( FALSE );
}

/*****************************************************************************/

gboolean on_qpsk_drawingarea_draw(
        GtkWidget *widget,
        cairo_t *cr,
        gpointer data) {
  if( qpsk_pixbuf != NULL )
  {
    Qpsk_Drawingarea_Draw( cr );
    return( TRUE );
  }
  else
    return( FALSE );
}

/*****************************************************************************/

void on_sdr_bw_entry_activate(GtkEntry *entry, gpointer data) {
  BW_Entry_Activate( entry );
}

/*****************************************************************************/

void on_sdr_freq_entry_activate(GtkEntry *entry, gpointer data) {
  /* Get center freq value in Hz, entry value is in kHz */
  double khz = atof( gtk_entry_get_text(entry) );
  rc_data.sdr_center_freq = (uint32_t)( khz * 1000.0 );
  if( isFlagSet(STATUS_SOAPYSDR_INIT) )
    SoapySDR_Set_Center_Freq( rc_data.sdr_center_freq );
}

/*****************************************************************************/

gboolean on_sig_level_drawingarea_draw(
        GtkWidget *widget,
        cairo_t *cr,
        gpointer data) {
  Draw_Level_Gauge( widget, cr, Signal_Level(NULL) );
  return( TRUE );
}

/*****************************************************************************/

gboolean on_sig_qual_drawingarea_draw(
        GtkWidget *widget,
        cairo_t *cr,
        gpointer data) {
  Draw_Level_Gauge( widget, cr, Sig_Quality() );
  return( TRUE );
}

/*****************************************************************************/

gboolean on_agc_gain_drawingarea_draw(
        GtkWidget *widget,
        cairo_t *cr,
        gpointer data) {
  Draw_Level_Gauge( widget, cr, Agc_Gain(NULL) );
  return( TRUE );
}

/*****************************************************************************/

gboolean on_pll_ave_drawingarea_draw(
        GtkWidget *widget,
        cairo_t *cr,
        gpointer data) {
  Draw_Level_Gauge( widget, cr, Pll_Average() );
  return( TRUE );
}

/*****************************************************************************/

void on_sdr_gain_scale_value_changed(GtkRange *range, gpointer data) {
  rc_data.tuner_gain = gtk_range_get_value( range );
  if( isFlagSet(STATUS_SOAPYSDR_INIT) &&
      isFlagClear(TUNER_GAIN_AUTO) )
    SoapySDR_Set_Tuner_Gain( rc_data.tuner_gain );
}

/*****************************************************************************/

void on_start_togglebutton_toggled(
        GtkToggleButton *togglebutton,
        gpointer data) {
  Start_Togglebutton_Toggled( togglebutton );
}

/*****************************************************************************/

void on_manual_agc_radiobutton_toggled(
        GtkToggleButton *togglebutton,
        gpointer data) {
  if( gtk_toggle_button_get_active(togglebutton) )
  {
    ClearFlag( TUNER_GAIN_AUTO );

    if( isFlagSet(STATUS_SOAPYSDR_INIT) )
      SoapySDR_Set_Tuner_Gain_Mode();

    GtkWidget *hscale =
      Builder_Get_Object( main_window_builder, "sdr_gain_hscale");
    gtk_range_set_value( GTK_RANGE(hscale), rc_data.tuner_gain );
  }
}

/*****************************************************************************/

void on_auto_agc_radiobutton_toggled(
        GtkToggleButton *togglebutton,
        gpointer data) {
  if( gtk_toggle_button_get_active(togglebutton) )
  {
    SetFlag( TUNER_GAIN_AUTO );
    if( isFlagSet(STATUS_SOAPYSDR_INIT) )
      SoapySDR_Set_Tuner_Gain_Mode();
  }
}
