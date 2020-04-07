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

#include "interface.h"

#include "../common/common.h"
#include "../common/shared.h"
#include "../sdr/filters.h"
#include "callback_func.h"
#include "utils.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <stddef.h>
#include <stdio.h>

/*****************************************************************************/

static void Gtk_Builder(GtkBuilder **builder, gchar **object_ids);

/*****************************************************************************/

/* Gtk_Builder()
 *
 * Returns a GtkBuilder with required objects from file
 */
static void Gtk_Builder(GtkBuilder **builder, gchar **object_ids) {
  GError *gerror = NULL;
  int ret = 0;

  /* Create a builder from object ids */
  *builder = gtk_builder_new();
  ret = (int)gtk_builder_add_objects_from_file(
      *builder, rc_data.glrpt_glade, object_ids, &gerror );
  if( !ret )
  {
    fprintf( stderr,
        "glrpt: Failed to add objects to builder:\n%s\n",
        gerror->message );
    exit( -1 );
  }

  /* Connect signals if gmodule is supported */
  if( !g_module_supported() )
  {
    fprintf( stderr, "glrpt: lib gmodule not supported\n" );
    exit( -1 );
  }
  gtk_builder_connect_signals( *builder, NULL );
}

/*****************************************************************************/

/* Builder_Get_Object()
 *
 * Gets a named object from the GtkBuilder builder object
 */
GtkWidget *Builder_Get_Object(GtkBuilder *builder, gchar *name) {
  GObject *object = gtk_builder_get_object( builder, name );
  if( object == NULL )
  {
    fprintf( stderr,
        "!! glrpt: builder failed to get named object: %s\n",
        name );
    exit( -1 );
  }

  return( GTK_WIDGET(object) );
}

/*****************************************************************************/

/*
 * These functions below create various widgets using GtkBuilder
 */
GtkWidget *create_main_window(GtkBuilder **builder) {
  gchar *object_ids[] = { MAIN_WINDOW_IDS };
  Gtk_Builder( builder, object_ids );
  GtkWidget *window =
    Builder_Get_Object( *builder, "main_window" );
  return( window );
}

/*****************************************************************************/

GtkWidget *create_popup_menu(GtkBuilder **builder) {
  gchar *object_ids[] = { POPUP_MENU_IDS };
  Gtk_Builder( builder, object_ids );
  GtkWidget *menu =
    Builder_Get_Object( *builder, "popup_menu" );
  return( menu );
}

/*****************************************************************************/

GtkWidget *create_timer_dialog(GtkBuilder **builder) {
  gchar *object_ids[] = { OPER_TIMER_DIALOG_IDS };
  Gtk_Builder( builder, object_ids );
  GtkWidget *dialog =
    Builder_Get_Object( *builder, "decode_timer_dialog" );
  return( dialog );
}

/*****************************************************************************/

GtkWidget *create_error_dialog(GtkBuilder **builder) {
  gchar *object_ids[] = { ERROR_DIALOG_IDS };
  Gtk_Builder( builder, object_ids );
  GtkWidget *dialog =
    Builder_Get_Object( *builder, "error_dialog" );
  return( dialog );
}

/*****************************************************************************/

GtkWidget *create_startstop_timer(GtkBuilder **builder) {
  gchar *object_ids[] = { AUTO_TIMER_DIALOG_IDS };
  Gtk_Builder( builder, object_ids );
  GtkWidget *dialog =
    Builder_Get_Object( *builder, "auto_timer_dialog" );
  return( dialog );
}

/*****************************************************************************/

GtkWidget *create_quit_dialog(GtkBuilder **builder) {
  gchar *object_ids[] = { QUIT_DIALOG_IDS };
  Gtk_Builder( builder, object_ids );
  GtkWidget *dialog =
    Builder_Get_Object( *builder, "quit_dialog" );
  return( dialog );
}

/*****************************************************************************/

/*  Initialize_Top_Window()
 *
 *  Initializes glrpt's top window
 */
void Initialize_Top_Window(void) {
  /* The scrolled window image container */
  gchar text[48];

  /* Show current satellite */
  GtkLabel *label = GTK_LABEL(
      Builder_Get_Object(main_window_builder, "satellite_label") );
  snprintf( text, sizeof(text), "%s LRPT", rc_data.satellite_name );
  gtk_label_set_text( label, text );

  /* Show Center_Freq to frequency entry */
  Enter_Center_Freq( rc_data.sdr_center_freq );

  /* Show Bandwidth to B/W entry */
  Enter_Filter_BW();

  /* Kill existing pixbuf */
  if( scaled_image_pixbuf != NULL )
  {
    g_object_unref( scaled_image_pixbuf );
    scaled_image_pixbuf = NULL;
  }

  /* Create new pixbuff for scaled images (+3 for white separator lines) */
  scaled_image_width   = (3 * METEOR_IMAGE_WIDTH) / (int)rc_data.image_scale + 3;
  scaled_image_height  = (int)rc_data.decode_timer * IMAGE_LINES_PERSEC;
  scaled_image_height /= (int)rc_data.image_scale;

  /* Create a pixbuf for the LRPT image display */
  scaled_image_pixbuf = gdk_pixbuf_new(
      GDK_COLORSPACE_RGB, FALSE, 8, scaled_image_width, scaled_image_height );

  /* Error, not enough memory */
  if( scaled_image_pixbuf == NULL)
  {
    Show_Message( "Memory allocation for pixbuf failed - Quit", "red" );
    Error_Dialog();
    return;
  }

  /* Get details of pixbuf */
  scaled_image_pixel_buf  = gdk_pixbuf_get_pixels(     scaled_image_pixbuf );
  scaled_image_rowstride  = gdk_pixbuf_get_rowstride(  scaled_image_pixbuf );
  scaled_image_n_channels = gdk_pixbuf_get_n_channels( scaled_image_pixbuf );

  /* Fill pixbuf with background color */
  gdk_pixbuf_fill( scaled_image_pixbuf, 0xaaaaaaff );

  /* Globalize image to be displayed */
  lrpt_image = Builder_Get_Object( main_window_builder, "lrpt_image" );

  /* Set lrpt image from pixbuff */
  gtk_image_set_from_pixbuf( GTK_IMAGE(lrpt_image), scaled_image_pixbuf );
  gtk_widget_show( lrpt_image );

  /* Set window size as required (minimal) */
  gtk_window_resize( GTK_WINDOW(main_window), 10, 10 );
}
