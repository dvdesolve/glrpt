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

#include "callback_func.h"

#include "../common/common.h"
#include "../common/shared.h"
#include "display.h"
#include "image.h"
#include "interface.h"
#include "../lrpt_decode/medet.h"
#include "../lrpt_demod/demod.h"
#include "../sdr/ifft.h"
#include "../sdr/SoapySDR.h"
#include "utils.h"

#include <cairo/cairo.h>
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/*------------------------------------------------------------------------*/

/* Error_Dialog()
 *
 * Opens an error dialog box
 */
  void
Error_Dialog( void )
{
  GtkBuilder *builder;
  if( !error_dialog )
  {
    error_dialog = create_error_dialog( &builder );
    gtk_widget_show( error_dialog );
    g_object_unref( builder );
  }

} /* Error_Dialog() */

/*------------------------------------------------------------------------*/

/* Cancel_Timer()
 *
 * Handles cancellation of decoder timer
 */
  gboolean
Cancel_Timer( gpointer data )
{
  alarm( 0 );
  Show_Message( "Cancelling Decode Timer", "orange" );
  ClearFlag( ENABLE_DECODE_TIMER );
  ClearFlag( START_STOP_TIMER );
  ClearFlag( ALARM_ACTION_START );

  gtk_toggle_button_set_active(
      GTK_TOGGLE_BUTTON(start_togglebutton), FALSE );

  Show_Message( "Setting Decode Timer to Default", "black" );
  rc_data.decode_timer = rc_data.default_timer;
  Initialize_Top_Window();

  return( FALSE );
} /* Cancel_Timer() */

/*------------------------------------------------------------------------*/

/* Sensitize_Menu_Item()
 *
 * Handles dynamic sensitivity changes to pop-up menu items
 */
  static void
Sensitize_Menu_Item( gchar *item_name, gboolean flag )
{
  GtkWidget *menu_item;
  menu_item = Builder_Get_Object( popup_menu_builder, item_name );
  gtk_widget_set_sensitive( menu_item, flag );

} /* Sensitize_Menu_Item() */

/*------------------------------------------------------------------------*/

/* Set_Check_Menu_Item()
 *
 * Sets pop-up check menu items active/inactive
 */
  void
Set_Check_Menu_Item( gchar *item_name, gboolean flag )
{
  GtkWidget *menu_item;
  menu_item = Builder_Get_Object( popup_menu_builder, item_name );
  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(menu_item), flag );

} /* Set_Check_Menu_Item() */

/*------------------------------------------------------------------------*/

/* Popup_Menu()
 *
 * Opens pop-up menu
 */
  void
Popup_Menu( void )
{
  if( isFlagSet(STATUS_PENDING) ) return;

  /* Initialize on first call */
  if( !popup_menu )
    popup_menu = create_popup_menu( &popup_menu_builder );

  /* If any action is running */
  if( isFlagSet(STATUS_FLAGS_ALL) )
  {
    Sensitize_Menu_Item( "select_satellite", FALSE );
    Sensitize_Menu_Item( "decode_timer_menuitem", FALSE );
    Sensitize_Menu_Item( "auto_timer_menuitem", FALSE );
  }
  else /* If no action is running */
  {
    if( isFlagClear(START_STOP_TIMER) )
    {
      Sensitize_Menu_Item( "decode_timer_menuitem", TRUE  );
      Sensitize_Menu_Item( "auto_timer_menuitem", TRUE  );
    }
    else
    {
      Sensitize_Menu_Item( "decode_timer_menuitem", FALSE );
      Sensitize_Menu_Item( "auto_timer_menuitem", FALSE );
    }

    Sensitize_Menu_Item( "select_satellite", TRUE  );
  } /* if( isFlagSet(STATUS_FLAGS_ALL) ) */

  /* Set action menu items according to state */
  if( isFlagSet(STATUS_RECEIVING) )
    Set_Check_Menu_Item( "start_receiver_menuitem", TRUE );
  else
    Set_Check_Menu_Item( "start_receiver_menuitem", FALSE );

  if( isFlagSet(STATUS_DECODING) )
    Set_Check_Menu_Item( "decode_images_menuitem", TRUE );
  else
    Set_Check_Menu_Item( "decode_images_menuitem", FALSE );

  if( isFlagSet(ENABLE_DECODE_TIMER) )
    Sensitize_Menu_Item( "cancel_timer_menuitem", TRUE  );
  else
    Sensitize_Menu_Item( "cancel_timer_menuitem", FALSE );

  /* Set check menu items according to config parameters */
  if( isFlagSet(IMAGE_OUT_COMBO) )
  {
    Set_Check_Menu_Item( "combine_menuitem", TRUE );
    if( isFlagSet(IMAGE_COLORIZE) )
      Set_Check_Menu_Item( "pseudo_menuitem", TRUE );
  }
  else
  {
    gboolean color = FALSE;
    if( isFlagSet(IMAGE_COLORIZE) ) color = TRUE;
    Set_Check_Menu_Item( "pseudo_menuitem", FALSE );
    if( color ) SetFlag( IMAGE_COLORIZE );
  }

  if( isFlagSet(IMAGE_OUT_SPLIT) )
    Set_Check_Menu_Item( "individual_menuitem", TRUE );

  if( isFlagSet(IMAGE_RAW) )
    Set_Check_Menu_Item( "raw_menuitem", TRUE );

  if( isFlagSet(IMAGE_NORMALIZE) )
  {
    Set_Check_Menu_Item( "normalize_menuitem", TRUE );
    if( isFlagSet(IMAGE_CLAHE) )
      Set_Check_Menu_Item( "clahe_menuitem", TRUE );
  }
  else
  {
    gboolean clahe = FALSE;
    if( isFlagSet(IMAGE_CLAHE) ) clahe = TRUE;
    Set_Check_Menu_Item( "clahe_menuitem", FALSE );
    if( clahe ) SetFlag( IMAGE_CLAHE );
  }

  if( isFlagSet(IMAGE_RECTIFY) )
    Set_Check_Menu_Item( "rectify_menuitem", TRUE );

  if( isFlagSet(IMAGE_SAVE_JPEG) )
    Set_Check_Menu_Item( "jpeg_menuitem", TRUE );

  if( isFlagSet(IMAGE_SAVE_PPGM) )
    Set_Check_Menu_Item( "pgm_menuitem", TRUE );

  gtk_menu_popup_at_pointer( GTK_MENU(popup_menu), NULL );

} /*  Popup_Menu() */

/*------------------------------------------------------------------------*/

/* Init_Reception()
 *
 * Initialize Reception of signal from Satellite
 */
  static gboolean
Init_Reception( void )
{
  /* Initialize semaphore */
  sem_init( &demod_semaphore, 0, 0 );

  /* Initialize SoapySDR device */
  if( !SoapySDR_Init() )
  {
    Show_Message( "Failed to Initialize SoapySDR", "red" );
    Error_Dialog();
    return( FALSE );
  }

  /* Create Demodulator object */
  Demod_Init();

  return( TRUE );
} /* Init_Reception() */

/*------------------------------------------------------------------------*/

/* Start_Togglebutton_Toggled()
 *
 * Handles the on_start_togglebutton_toggled CB
 */
  void
Start_Togglebutton_Toggled( GtkToggleButton *togglebutton )
{
  /* Start SDR Receeiver if not satrted already */
  if( gtk_toggle_button_get_active(togglebutton) &&
      isFlagClear(STATUS_RECEIVING) &&
      isFlagClear(STATUS_PENDING) )
  {
    Set_Check_Menu_Item( "decode_images_menuitem",  TRUE );
    Set_Check_Menu_Item( "start_receiver_menuitem", TRUE );
  }

  if( !gtk_toggle_button_get_active(togglebutton) )
  {
    Set_Check_Menu_Item( "start_receiver_menuitem", FALSE );
  }

} /* Start_Togglebutton_Toggled() */

/*------------------------------------------------------------------------*/

/* Start_Receiver_Menuitem_Toggled()
 *
 * Handles the on_start_receiver_menuitem_toggled CB
 */
  void
Start_Receiver_Menuitem_Toggled( GtkCheckMenuItem *menuitem )
{
  /* Start SDR Receeiver if not satrted already */
  if( gtk_check_menu_item_get_active(menuitem) &&
      isFlagClear(STATUS_RECEIVING) &&
      isFlagClear(STATUS_PENDING) )
  {
    /* Start SDR Receiver and Demodulator.
     * Decoder will start if enabled by user
     */

    /* Initialize SDR Receiver and QPSK demodulator */
    SetFlag( STATUS_PENDING );
    if( !Init_Reception() )
    {
      ClearFlag( STATUS_PENDING );
      return;
    }

    /* Activate the SoapySDR Receive Stream */
    SetFlag( STATUS_RECEIVING );
    if( !SoapySDR_Activate_Stream() )
    {
      ClearFlag( STATUS_RECEIVING );
      return;
    }

    /* Start demodulator by idle callback */
    ClearFlag(STATUS_PENDING);
    g_idle_add( Demodulator_Run, NULL );

    /* Display Device Driver in use */
    char mesg[MESG_SIZE];
    snprintf( mesg, sizeof(mesg),
        "Decoding from Device \"%s\"", rc_data.device_driver );
    Show_Message( mesg, "green" );
  } /* if( gtk_check_menu_item_get_active(menuitem) && */

  if( !gtk_check_menu_item_get_active(menuitem) &&
      isFlagSet(STATUS_RECEIVING) )
  {
    if( rc_data.psk_mode == IDOQPSK )
      SetFlag( STATUS_IDOQPSK_STOP );
    else
      ClearFlag( STATUS_RECEIVING );
  }

} /* Start_Receiver_Menuitem_Toggled() */

/*------------------------------------------------------------------------*/

/* Decode_Images_Menuitem_Activate()
 *
 * Handles the on_decode_images_menuitem_toggled CB
 */
  void
Decode_Images_Menuitem_Toggled( GtkCheckMenuItem *menuitem )
{
  /* Start LRPT Image Decoder if not already started */
  if( gtk_check_menu_item_get_active(menuitem) &&
      isFlagClear(STATUS_DECODING) )
  {
    /* Reset Scaled Images display */
    Display_Scaled_Image( NULL, 0, 0 );

    /* Initialize Meteor Image Decoder */
    Medet_Init();

    /* Start Timer if enabled */
    if( isFlagSet(ENABLE_DECODE_TIMER) )
    {
      /* Message string buffer */
      char mesg[MESG_SIZE];

      snprintf( mesg, MESG_SIZE,
          "Decode Timer Started: %u sec", rc_data.decode_timer );
      Show_Message( mesg, "orange" );
      SetFlag( ALARM_ACTION_STOP );
      alarm( rc_data.decode_timer );
    }

    Show_Message( "Decoding of LRPT Images Started", "black" );
    SetFlag( STATUS_DECODING );
    return;
  } /* if( gtk_check_menu_item_get_active(menuitem) && */

  /* Stop LRPT Image Decoder if it is running */
  if( !gtk_check_menu_item_get_active(menuitem) &&
      isFlagSet(STATUS_DECODING) )
  {
    ClearFlag( STATUS_DECODING );
    Show_Message( "Decoder Timer Cancelled", "orange" );
    alarm( 0 );
    ClearFlag( ALARM_ACTION_STOP );
    Medet_Deinit();

    Show_Message( "Decoding of LRPT Images Stopped", "black" );
    Display_Icon( frame_icon, "gtk-no" );
    ClearFlag( FRAME_OK_ICON );
  } /* if( !gtk_check_menu_item_get_active(menuitem) && */

} /* Decode_Images_Menuitem_Toggled() */

/*------------------------------------------------------------------------*/

/* Alarm_Action()
 *
 * Handles the SIGALRM timer signal
 */
  void
Alarm_Action( void )
{
  /* Start Receive/Decode Operation */
  if( isFlagSet(ALARM_ACTION_START) &&
      isFlagClear(STATUS_RECEIVING) )
  {
    /* Start SDR Receiver and Demodulator/Decoder */
    Show_Message( "Starting Receiver & Decoder", "black" );

    /* Set up alarm to stop ongoing action */
    ClearFlag( ALARM_ACTION_START );
    SetFlag( ALARM_ACTION_STOP );
    alarm( rc_data.decode_timer );

    /* Reset Scaled Images display */
    Display_Scaled_Image( NULL, 0, 0 );

    /* Initialize Meteor Image Decoder */
    Medet_Init();
    SetFlag( STATUS_DECODING );

    /* Initialize SDR Receiver and QPSK demodulator */
    SetFlag( STATUS_PENDING );
    if( !Init_Reception() )
    {
      ClearFlag( STATUS_PENDING );
      return;
    }

    /* Activate the SoapySDR Receive Stream */
    SetFlag( STATUS_RECEIVING );
    if( !SoapySDR_Activate_Stream() )
    {
      ClearFlag( STATUS_RECEIVING );
      return;
    }

    /* Display Device Driver in use */
    char mesg[MESG_SIZE];
    snprintf( mesg, sizeof(mesg),
        "Decoding from %s Receiver", rc_data.device_driver );
    Show_Message( mesg, "black" );

    /* Start demodulator by idle callback */
    ClearFlag(STATUS_PENDING);
    g_idle_add( Demodulator_Run, NULL );

    return;
  } /* if( isFlagSet(ALARM_ACTION_START) ) */

  /* Stop Receive/Decode Operation */
  if( isFlagSet(ALARM_ACTION_STOP) )
  {
    Show_Message( "Decode Timer Expired", "orange" );
    ClearFlag( ALARM_ACTION_STOP );
    alarm( 0 );

    if( rc_data.psk_mode == IDOQPSK )
    {
      SetFlag( STATUS_IDOQPSK_STOP );
      return;
    }

    ClearFlag( STATUS_RECEIVING );
    ClearFlag( STATUS_DECODING );
    Medet_Deinit();

    return;
  }

} /* Alarm_Action() */

/*------------------------------------------------------------------------*/

/* Decode_Timer_Setup()
 *
 * Handles on_timeout_okbutton_clicked CB
 */
  void
Decode_Timer_Setup( void )
{
  GtkWidget *spinbutton;
  char mesg[MESG_SIZE];

  /* Set decode timer in seconds */
  spinbutton =
    Builder_Get_Object( decode_timer_dialog_builder, "decode_timer_spinbutton" );
  rc_data.decode_timer = (uint32_t)
    ( 60 * gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton)) );

  /* Warn if duration is too long */
  if( rc_data.decode_timer > MAX_OPERATION_TIME )
  {
    snprintf( mesg, MESG_SIZE,
        "Decode Timer (%u sec) excessive?", rc_data.decode_timer );
    Show_Message( mesg, "red" );
    Error_Dialog();
  }
  snprintf( mesg, MESG_SIZE,
      "Decode Timer set to %u sec", rc_data.decode_timer );
  Show_Message( mesg, "orange" );

  gtk_widget_destroy( decode_timer_dialog );
  Initialize_Top_Window();
  SetFlag( ENABLE_DECODE_TIMER );

} /* Decode_Timer_Setup() */

/*------------------------------------------------------------------------*/

/* Auto_Timer_OK_Clicked()
 *
 * Handles the on_start_stop_ok_clicked CB
 *
 */
  void
Auto_Timer_OK_Clicked( void )
{
  uint
    start_hrs,  /* Start time hours   */
    start_min,  /* Start time minutes */
    stop_hrs,   /* Stop time hours    */
    stop_min,   /* Stop time minutes  */
    sleep_sec,  /* Sleep time in sec  */
    stop_sec,   /* Stop time in sec since 00:00 hrs  */
    start_sec,  /* Start time in sec since 00:00 hrs */
    time_sec;   /* Time now in sec since 00:00 hrs   */

  /* Message string buffer */
  char mesg[MESG_SIZE];

  /* Used to read real time */
  struct tm time_now;
  time_t t;


  /* Extract hours and minutes of start time */
  start_hrs = (uint32_t)atoi( gtk_entry_get_text(
        GTK_ENTRY(Builder_Get_Object(auto_timer_dialog_builder, "auto_start_hrs"))) );
  start_min = (uint32_t)atoi( gtk_entry_get_text(
        GTK_ENTRY(Builder_Get_Object(auto_timer_dialog_builder, "auto_start_min"))) );

  /* Time now */
  t = time( &t );
  time_now = *gmtime( &t );
  time_sec = (uint32_t)
    (time_now.tm_hour * 3600 + time_now.tm_min * 60 + time_now.tm_sec);

  /* Calculate action start setting for alarm */
  start_sec = start_hrs * 3600 + start_min * 60;
  if( start_sec < time_sec ) start_sec += 86400; /* Next day */
  sleep_sec = start_sec - time_sec;

  /* Extract hours and minutes of stop time */
  stop_hrs = (uint32_t)atoi( gtk_entry_get_text(
        GTK_ENTRY(Builder_Get_Object(auto_timer_dialog_builder, "auto_stop_hrs"))) );
  stop_min = (uint32_t)atoi( gtk_entry_get_text(
        GTK_ENTRY(Builder_Get_Object(auto_timer_dialog_builder, "auto_stop_min"))) );

  /* Calculate action stop alarm setting */
  stop_sec  = stop_hrs * 3600 + stop_min * 60;
  if( stop_sec < time_sec ) stop_sec += 86400; /* Next day */
  stop_sec -= time_sec;

  /* Data sanity check */
  if( stop_sec <= sleep_sec )
  {
    Show_Message( "Stop time ahead of Start time", "red" );
    Error_Dialog();
    return;
  }

  /* Difference between start-stop times in sec */
  rc_data.decode_timer = (uint32_t)( stop_sec - sleep_sec );

  /* Data sanity check */
  if( rc_data.decode_timer > MAX_OPERATION_TIME )
  {
    snprintf( mesg, MESG_SIZE,
        "Decode Timer (%u sec) excessive?", rc_data.decode_timer );
    Show_Message( mesg, "red" );
    Error_Dialog();
  }

  /* Notify sleeping */
  snprintf( mesg, MESG_SIZE,
      "Suspended till %02u:%02u UTC\n"\
        "Decode Timer set to %u sec",
      start_hrs, start_min, rc_data.decode_timer );
  Show_Message( mesg, "black" );

  /* Set sleep flag and wakeup action */
  alarm( sleep_sec );
  SetFlag( START_STOP_TIMER );
  SetFlag( ALARM_ACTION_START );
  SetFlag( ENABLE_DECODE_TIMER );

  Initialize_Top_Window();
  gtk_widget_destroy( auto_timer_dialog );

} /* Auto_Timer_OK_Clicked() */

/*------------------------------------------------------------------------*/

/* Hours_Entry()
 *
 * Handles on_hrs_entry_changed CB
 */
  void
Hours_Entry( GtkEditable *editable )
{
  gchar buff[3];
  uint32_t idx, len;
  int ent;

  /* Get user entry */
  Strlcpy( buff, gtk_entry_get_text(GTK_ENTRY(editable)), sizeof(buff) );
  len = (uint32_t)strlen( buff );

  /* Reject non-numeric entries */
  for( idx = 0; idx < len; idx++ )
    if( (buff[idx] < '0') || (buff[idx] > '9') )
    {
      Show_Message( "Non-numeric entry", "red" );
      Error_Dialog();
      return;
    }

  /* Reject invalid entries */
  ent = atoi( gtk_entry_get_text(GTK_ENTRY(editable)) );
  if( (ent < 0) || (ent > 23) )
  {
    Show_Message( "Value out of range", "red" );
    Error_Dialog();
    return;
  }

} /* Hours_Entry() */

/*------------------------------------------------------------------------*/

/* Minutes_Entry()
 *
 * Handles the on_min_entry_changed CB
 */
  void
Minutes_Entry( GtkEditable *editable )
{
  gchar buff[3];
  uint32_t idx, len;
  int ent;

  /* Get user entry */
  Strlcpy( buff, gtk_entry_get_text(GTK_ENTRY(editable)), sizeof(buff) );
  len = (uint32_t)strlen( buff );

  /* Reject non-numeric entries */
  for( idx = 0; idx < len; idx++ )
    if( (buff[idx] < '0') || (buff[idx] > '9') )
    {
      Show_Message( "Non-numeric entry", "red" );
      Error_Dialog();
      return;
    }

  /* Reject invalid entries */
  ent = atoi( gtk_entry_get_text(GTK_ENTRY(editable)) );
  if( (ent < 0) || (ent > 59) )
  {
    Show_Message( "Value out of range", "red" );
    Error_Dialog();
    return;
  }

} /* Minutes_Entry() */

/*------------------------------------------------------------------------*/

/* Enter_Center_Freq()
 *
 * Enters the center frequency to relevant
 * entry widget and displays in messages
 */
  void
Enter_Center_Freq( uint32_t freq )
{
  char text[12];

  /* Get center freq value in Hz, entry value is in kHz */
  GtkEntry *entry = GTK_ENTRY(
      Builder_Get_Object(main_window_builder, "sdr_freq_entry") );
  snprintf( text, sizeof(text), "%8.1f", (double)freq / 1000.0 );
  gtk_entry_set_text( entry, text );

} /* Enter_Center_Freq() */

/*------------------------------------------------------------------------*/

/* Fft_Drawingarea_Size_Alloc()
 *
 * Initializes the waterfall drawing area pixbuf
 */
  void
Fft_Drawingarea_Size_Alloc( GtkAllocation *allocation )
{
  /* Destroy existing pixbuff */
  if( wfall_pixbuf != NULL )
  {
    g_object_unref( G_OBJECT(wfall_pixbuf) );
    wfall_pixbuf = NULL;
  }

  /* Create waterfall pixbuf */
  wfall_pixbuf = gdk_pixbuf_new(
      GDK_COLORSPACE_RGB, FALSE, 8,
      allocation->width, allocation->height );
  if( wfall_pixbuf == NULL )
  {
    Show_Message( "Failed creating waterfall pixbuf", "red" );
    return;
  }

  /* Get waterfall pixbuf details */
  wfall_pixels = gdk_pixbuf_get_pixels( wfall_pixbuf );
  wfall_width  = gdk_pixbuf_get_width ( wfall_pixbuf );
  wfall_height = gdk_pixbuf_get_height( wfall_pixbuf );
  wfall_rowstride  = gdk_pixbuf_get_rowstride( wfall_pixbuf );
  wfall_n_channels = gdk_pixbuf_get_n_channels( wfall_pixbuf );
  gdk_pixbuf_fill( wfall_pixbuf, 0 );

  /* Initialize ifft. Waterfall with is an odd number
   * to provide a center line. IFFT requires a width
   * that is a power of 2 */
  if( ! Initialize_IFFT((int16_t)wfall_width + 1) )
    return;

} /* Fft_Drawingarea_Size_Alloc() */

/*------------------------------------------------------------------------*/

/* Qpsk_Drawingarea_Size_Alloc()
 *
 * Initializes the QPSK constellation drawing area pixbuf
 */
  void
Qpsk_Drawingarea_Size_Alloc( GtkAllocation *allocation )
{
  /* Destroy existing pixbuff */
  if( qpsk_pixbuf != NULL )
  {
    g_object_unref( G_OBJECT(qpsk_pixbuf) );
    qpsk_pixbuf = NULL;
  }

  /* Create waterfall pixbuf */
  qpsk_pixbuf = gdk_pixbuf_new(
      GDK_COLORSPACE_RGB, FALSE, 8, allocation->width, allocation->height );
  if( qpsk_pixbuf == NULL )
  {
    Show_Message( "Failed creating QPSK Const pixbuf", "red" );
    return;
  }

  /* Get waterfall pixbuf details */
  qpsk_pixels = gdk_pixbuf_get_pixels( qpsk_pixbuf );
  qpsk_width  = gdk_pixbuf_get_width ( qpsk_pixbuf );
  qpsk_height = gdk_pixbuf_get_height( qpsk_pixbuf );
  qpsk_center_x   = qpsk_width  / 2;
  qpsk_center_y   = qpsk_height / 2;
  qpsk_rowstride  = gdk_pixbuf_get_rowstride( qpsk_pixbuf );
  qpsk_n_channels = gdk_pixbuf_get_n_channels( qpsk_pixbuf );

  gdk_pixbuf_fill( qpsk_pixbuf, 0 );

} /* Qpsk_Drawingarea_Size_Alloc() */

/*------------------------------------------------------------------------*/

/* Qpsk_Drawingarea_Draw()
 *
 * Draws the QPSK Constellation drawing area
 */
  void
Qpsk_Drawingarea_Draw( cairo_t *cr )
{
  /* Draw the QPSK constellation */
  gdk_cairo_set_source_pixbuf( cr, qpsk_pixbuf, 0.0, 0.0 );
  cairo_paint( cr );

  /* Draw lines to define the four quadeants */
  cairo_set_source_rgb( cr, 1.0, 1.0, 1.0 );
  cairo_set_line_width( cr, 1.0 );
  double x2 = (double)qpsk_width  / 2.0;
  double y2 = (double)qpsk_height / 2.0;
  cairo_move_to( cr, x2, 0.0 );
  cairo_line_to( cr, x2, (double)qpsk_height );
  cairo_move_to( cr, 0.0, y2 );
  cairo_line_to( cr, (double)qpsk_width, y2 );
  cairo_stroke( cr );

  /* Fill pixbuf with background color */
  gdk_pixbuf_fill( qpsk_pixbuf, 0x000000ff );

} /* Qpsk_Drawingarea_Draw() */

/*------------------------------------------------------------------------*/

/* BW_Entry_Activate()
 *
 * Handles the activate callback for bandwidth entry
 */
  void
BW_Entry_Activate( GtkEntry *entry )
{

  /* Prevent filter re-initialization when in use */
  if( isFlagSet(STATUS_SOAPYSDR_INIT) )
  {
    Show_Message( "Cannot change Filter Bandwidth", "red" );
    Show_Message( "Filter is in use by SoapySDR", "red" );
    Error_Dialog();
    return;
  }

  /* Get bandwidth value in Hz and set */
  rc_data.sdr_filter_bw =
    (uint32_t)( 1000 * atoi(gtk_entry_get_text(entry)) );

  /* Check low pass filter bandwidth is in range */
  if( (rc_data.sdr_filter_bw < MIN_BANDWIDTH) ||
      (rc_data.sdr_filter_bw > MAX_BANDWIDTH) )
  {
    Show_Message( "Invalid Roofing Filter Bandwidth", "red" );
    Error_Dialog();
    return;
  }

  /* Show Bandwidth in messages */
  char text[MESG_SIZE];
  snprintf( text, sizeof(text),
      "Low Pass Filter B/W %u kHz", rc_data.sdr_filter_bw / 1000 );
  Show_Message( text, "black" );

} /* BW_Entry_Activate() */
