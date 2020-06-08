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

#include "rc_config.h"

#include "../common/common.h"
#include "../common/shared.h"
#include "callback_func.h"
#include "callbacks.h"
#include "interface.h"
#include "utils.h"

#include <glib-object.h>
#include <gtk/gtk.h>

#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************/

/* Special characters */
#define LF   0x0A /* Line Feed */
#define CR   0x0D /* Carriage Return */
#define HT   0x09 /* Horizontal Tab  */

/* Max length of lines in config file */
#define MAX_CONFIG_STRLEN   80

/*****************************************************************************/

static int Load_Line(char *buff, FILE *pfile, const char *mesg);

/*****************************************************************************/

static int found_cfg = 0;

/*****************************************************************************/

/* Load_Line()
 *
 * Loads a line from a file, aborts on failure. Lines beginning
 * with a '#' are ignored as comments. At the end of file EOF
 * is returned. Lines assumed maximum 80 characters long.
 */
static int Load_Line(char *buff, FILE *pfile, const char *mesg) {
  int
    num_chr, /* Number of characters read, excluding lf/cr */
    chr;     /* Character read by getc() */
  char error_mesg[MESG_SIZE];

  /* Prepare error message */
  snprintf( error_mesg, MESG_SIZE,
      "Error reading %s\n"\
        "Premature EOF (End Of File)", mesg );

  /* Clear buffer at start */
  buff[0] = '\0';
  num_chr = 0;

  /* Get next character, return error if chr = EOF */
  if( (chr = fgetc(pfile)) == EOF )
  {
    fprintf( stderr, "glrpt: %s\n", error_mesg );
    fclose( pfile );
    Show_Message( error_mesg, "red" );
    Error_Dialog();
    return( EOF );
  }

  /* Ignore commented lines and eol/cr and tab */
  while(
      (chr == '#') ||
      (chr == HT ) ||
      (chr == CR ) ||
      (chr == LF ) )
  {
    /* Go to the end of line (look for LF or CR) */
    while( (chr != CR) && (chr != LF) )
      /* Get next character, return error if chr = EOF */
      if( (chr = fgetc(pfile)) == EOF )
      {
        fprintf( stderr, "glrpt: %s\n", error_mesg );
        fclose( pfile );
        Show_Message( error_mesg, "red" );
        Error_Dialog();
        return( EOF );
      }

    /* Dump any CR/LF remaining */
    while( (chr == CR) || (chr == LF) )
      /* Get next character, return error if chr = EOF */
      if( (chr = fgetc(pfile)) == EOF )
      {
        fprintf( stderr, "glrpt: %s\n", error_mesg );
        fclose( pfile );
        Show_Message( error_mesg, "red" );
        Error_Dialog();
        return( EOF );
      }

  } /* End of while( (chr == '#') || ... */

  /* Continue reading characters from file till
   * number of characters = 80 or EOF or CR/LF */
  while( num_chr < MAX_CONFIG_STRLEN )
  {
    /* If LF/CR reached before filling buffer, return line */
    if( (chr == LF) || (chr == CR) ) break;

    /* Enter new character to line buffer */
    buff[num_chr++] = (char)chr;

    /* Get next character */
    if( (chr = fgetc(pfile)) == EOF )
    {
      /* Terminate buffer as a string if chr = EOF */
      buff[num_chr] = '\0';
      return( SUCCESS );
    }

    /* Abort if end of line not reached at 80 char. */
    if( (num_chr == 80) && (chr != LF) && (chr != CR) )
    {
      /* Terminate buffer as a string */
      buff[num_chr] = '\0';
      snprintf( error_mesg, MESG_SIZE,
          "Error reading %s\n"\
            "Line longer than 80 characters", mesg );
      fprintf( stderr, "glrpt: %s\n%s\n", error_mesg, buff );
      fclose( pfile );
      Show_Message( error_mesg, "red" );
      Error_Dialog();
      return( ERROR );
    }

  } /* End of while( num_chr < max_chr ) */

  /* Terminate buffer as a string */
  buff[num_chr] = '\0';

  return( SUCCESS );
}

/*****************************************************************************/

/* Load_Config()
 *
 * Loads the glrptrc configuration file
 */
bool Load_Config(void) {
  char
    rc_fpath[MESG_SIZE], /* File path to glrptrc */
    line[MAX_CONFIG_STRLEN + 1] = {0}; /* Buffer for Load_Line */

  /* Config file pointer */
  FILE *glrptrc;

  int idx;


  /* Setup file path to glrptrc and working dir */
  snprintf( rc_fpath, sizeof(rc_fpath),
      "%s/%s.cfg", rc_data.glrpt_cfgs, rc_data.satellite_name );

  /* Open glrptrc file */
  if( !found_cfg || !Open_File(&glrptrc, rc_fpath, "r") )
  {
    Show_Message( "Failed to open configuration file", "red" );
    Error_Dialog();
    return( false );
  }

  /*** Read runtime configuration data ***/

  /*** SDR Receiver configuration data ***/
  /* Read SDR Receiver SoapySDR Device Driver to use */
  if( Load_Line(line, glrptrc, "SoapySDR Device Driver") != SUCCESS )
    return( false );
  Strlcpy( rc_data.device_driver, line, sizeof(rc_data.device_driver) );
  if( strcmp(rc_data.device_driver, "auto") == 0 )
    SetFlag( AUTO_DETECT_SDR );

  /* Read SoapySDR Device Index, abort if EOF */
  if( Load_Line(line, glrptrc, "SDR Device Index") != SUCCESS )
    return( false );
  idx = atoi( line );
  if( (idx < 0) || (idx > 8) )
  {
    Show_Message(
        "Invalid SoapySDR Device Index\n"\
          "Quit and correct glrptrc", "red" );
    Error_Dialog();
    return( false );
  }
  rc_data.device_index = (uint32_t)idx;

  /* Read Low Pass Filter Bandwidth, abort if EOF */
  if( Load_Line(line, glrptrc, "Roofing Filter Bandwidth") != SUCCESS )
    return( false );
  rc_data.sdr_filter_bw = (uint32_t)( atoi(line) );

  /*** Read Device configuration data ***/
  /* Read Manual AGC Setting, abort if EOF */
  if( Load_Line(line, glrptrc, "Manual Gain Setting") != SUCCESS )
    return( false );
  rc_data.tuner_gain = atof( line );
  if( rc_data.tuner_gain > 100.0 )
  {
    rc_data.tuner_gain = 100.0;
    Show_Message(
        "Invalid Manual Gain Setting\n"\
          "Assuming a value of 100%", "red" );
    Error_Dialog();
    return( false );
  }

  /* Read Frequency Correction Factor, abort if EOF */
  if( Load_Line(line, glrptrc, "Frequency Correction Factor") != SUCCESS )
    return( false );
  rc_data.freq_correction = atoi( line );
  if( abs(rc_data.freq_correction) > 100 )
  {
    Show_Message(
        "Invalid Frequency Correction Factor\n"\
          "Quit and correct glrptrc", "red" );
    Error_Dialog();
    return( false );
  }

  /*** Image Decoding configuration data ***/
  /* Read Satellite Frequency in kHz, abort if EOF */
  if( Load_Line(line, glrptrc, "Satellite Frequency kHz") != SUCCESS )
    return( false );
  rc_data.sdr_center_freq = (uint32_t)atoi( line ) * 1000;

  /* Read default decode duration, abort if EOF */
  if( Load_Line(line, glrptrc, "Image Decoding Duration") != SUCCESS )
    return( false );
  rc_data.default_timer = (uint32_t)( atoi(line) );
  if( !rc_data.decode_timer )
    rc_data.decode_timer = rc_data.default_timer;

  /* Warn if decoding duration is too long */
  if( rc_data.decode_timer > MAX_OPERATION_TIME )
  {
    char mesg[MESG_SIZE];
    snprintf( mesg, sizeof(mesg),
        "Default decoding duration specified\n"\
          "in glrptrc (%u sec) seems excessive\n",
        rc_data.decode_timer );
    Show_Message( mesg, "red" );
  }

  /* Read LRPT image scale factor, abort if EOF */
  if( Load_Line(line, glrptrc, "Image Scale Factor") != SUCCESS )
    return( false );
  rc_data.image_scale = (uint32_t)( atoi(line) );

  /* LRPT Demodulator Parameters */
  /* Read RRC Filter Order, abort if EOF */
  if( Load_Line(line, glrptrc, "RRC Filter Order") != SUCCESS )
    return( false );
  rc_data.rrc_order = (uint32_t)( atoi(line) );

  /* Read RRC Filter alpha factor, abort if EOF */
  if( Load_Line(line, glrptrc, "RRC Filter alpha factor") != SUCCESS )
    return( false );
  rc_data.rrc_alpha = atof( line );

  /* Read Costas PLL Loop Bandwidth, abort if EOF */
  if( Load_Line(line, glrptrc, "Costas PLL Loop Bandwidth") != SUCCESS )
    return( false );
  rc_data.costas_bandwidth = atof( line );

  /* Read Costas PLL Locked Threshold, abort if EOF */
  if( Load_Line(line, glrptrc, "Costas PLL Locked Threshold") != SUCCESS )
    return( false );
  rc_data.pll_locked   = atof( line );
  rc_data.pll_unlocked = rc_data.pll_locked * 1.03;

  /* Read Transmitter Modulation Mode, abort if EOF */
  if( Load_Line(line, glrptrc, "Transmitter Modulation Mode") != SUCCESS )
    return( false );
  rc_data.psk_mode = (uint8_t)( atoi(line) );

  /* Read Transmitter QPSK Symbol Rate, abort if EOF */
  if( Load_Line(line, glrptrc, "Transmitter QPSK Symbol Rate") != SUCCESS )
    return( false );
  rc_data.symbol_rate = (uint32_t)( atoi(line) );

  /* Read Demodulator Interpolation Factor, abort if EOF */
  if( Load_Line(line, glrptrc, "Demodulator Interpolation Factor") != SUCCESS )
    return( false );
  rc_data.interp_factor = (uint32_t)( atoi(line) );

  /* Read LRPT Decoder Output Mode, abort if EOF */
  if( Load_Line(line, glrptrc, "LRPT Decoder Output Mode") != SUCCESS )
    return( false );
  switch( atoi(line) )
  {
    case OUT_COMBO:
      SetFlag( IMAGE_OUT_COMBO );
      break;

    case OUT_SPLIT:
      SetFlag( IMAGE_OUT_SPLIT );
      break;

    case OUT_BOTH:
      SetFlag( IMAGE_OUT_COMBO );
      SetFlag( IMAGE_OUT_SPLIT );
      break;

    default:
      SetFlag( IMAGE_OUT_COMBO );
      SetFlag( IMAGE_OUT_SPLIT );
      Show_Message(
          "Image Output Mode option invalid\n"
            "Assuming Both (Split and Combo)", "red" );
  }

  /* Read LRPT Image Save file type, abort if EOF */
  if( Load_Line(line, glrptrc, "Save As image file type") != SUCCESS )
    return( false );
  switch( atoi(line) )
  {
    case SAVEAS_JPEG:
      SetFlag( IMAGE_SAVE_JPEG );
      break;

    case SAVEAS_PGM:
      SetFlag( IMAGE_SAVE_PPGM );
      break;

    case SAVEAS_BOTH:
      SetFlag( IMAGE_SAVE_JPEG );
      SetFlag( IMAGE_SAVE_PPGM );
      break;

    default:
      SetFlag( IMAGE_SAVE_PPGM );
      SetFlag( IMAGE_SAVE_PPGM );
      Show_Message(
          "Image Save As option invalid\n"
            "Assuming Both (JPEG and PGM)", "red" );
  }

  /* Read JPEG Quality Factor, abort if EOF */
  /* TODO seems like mess-up with type casting */
  if( Load_Line(line, glrptrc, "JPEG Quality Factor") != SUCCESS )
    return( false );
  rc_data.jpeg_quality = atoi(line);

  /* Read LRPT Decoder Image Raw flag, abort if EOF */
  if( Load_Line(line, glrptrc, "LRPT Decoder Image Raw flag") != SUCCESS )
    return( false );
  if( atoi(line) ) SetFlag( IMAGE_RAW );

  /* Read LRPT Decoder Image Normalize flag, abort if EOF */
  if( Load_Line(line, glrptrc, "LRPT Decoder Image Normalize flag") != SUCCESS )
    return( false );
  if( atoi(line) ) SetFlag( IMAGE_NORMALIZE );

  /* Read LRPT Decoder Image CLAHE flag, abort if EOF */
  if( Load_Line(line, glrptrc, "LRPT Decoder Image CLAHE flag") != SUCCESS )
    return( false );
  if( atoi(line) ) SetFlag( IMAGE_CLAHE );

  /* Read LRPT Decoder Image Rectify flag, abort if EOF */
  if( Load_Line(line, glrptrc, "LRPT Decoder Image Rectify flag") != SUCCESS )
    return( false );
  rc_data.rectify_function = (uint8_t)atoi( line );
  if( rc_data.rectify_function > 2 )
  {
    Show_Message( "Invalid Rectify Function. Assuming 1", "red" );
    rc_data.rectify_function = 1;
  }
  if( rc_data.rectify_function )
    SetFlag( IMAGE_RECTIFY );

  /* Read LRPT Decoder Image Colorize flag, abort if EOF */
  if( Load_Line(line, glrptrc, "LRPT Decoder Image Colorize flag") != SUCCESS )
    return( false );
  if( atoi(line) ) SetFlag( IMAGE_COLORIZE );

  /* Read LRPT Decoder Channel 0 APID, abort if EOF */
  if( Load_Line(line, glrptrc, "LRPT Decoder Red APID") != SUCCESS )
    return( false );
  rc_data.apid[0] = (uint8_t)( atoi(line) );

  /* Read LRPT Decoder Channel 1 APID, abort if EOF */
  if( Load_Line(line, glrptrc, "LRPT Decoder Green APID") != SUCCESS )
    return( false );
  rc_data.apid[1] = (uint8_t)( atoi(line) );

  /* Read LRPT Decoder Channel 2 APID, abort if EOF */
  if( Load_Line(line, glrptrc, "LRPT Decoder Blue APID") != SUCCESS )
    return( false );
  rc_data.apid[2] = (uint8_t)( atoi(line) );

  /* Read LRPT Decoder Channels to be used for the combined
   * color image's red, green and blue channels, abort if EOF */
  if( Load_Line(line, glrptrc, "Combined Color Image Channel Numbers") != SUCCESS )
    return( false );
  for( idx = 0; idx < CHANNEL_IMAGE_NUM; idx++ )
  {
    rc_data.color_channel[idx] = (uint8_t)( atoi(line + 2 * idx) );
    if( rc_data.color_channel[idx] >= CHANNEL_IMAGE_NUM )
    {
      Show_Message(
          "Channel Number for Combined\n"
          "Color Image out of Range", "red" );
      return( false );
    }
  }

  /* Read image APIDs to invert palette, abort if EOF */
  if( Load_Line(line, glrptrc, "Invert Palette APIDs") != SUCCESS )
    return( false );
  char *nptr = line, *endptr = NULL;
  for( idx = 0; idx < 3; idx++ )
  {
    rc_data.invert_palette[idx] = (uint32_t)( strtol(nptr, &endptr, 10) );
    nptr = ++endptr;
  }

  /* Read Red Channel Normalization Range, abort if EOF */
  if( Load_Line(line, glrptrc, "Red Channel Normalization Range") != SUCCESS )
    return( false );
  rc_data.norm_range[RED][NORM_RANGE_BLACK] = (uint8_t)( atoi(line) );
  idx = 0;
  while( line[idx++] != '-' );
  rc_data.norm_range[RED][NORM_RANGE_WHITE] = (uint8_t)( atoi(&line[idx]) );

  /* Read Green Channel Normalization Range, abort if EOF */
  if( Load_Line(line, glrptrc, "Green Channel Normalization Range") != SUCCESS )
    return( false );
  rc_data.norm_range[GREEN][NORM_RANGE_BLACK] = (uint8_t)atoi( line );
  idx = 0;
  while( line[idx++] != '-' );
  rc_data.norm_range[GREEN][NORM_RANGE_WHITE] = (uint8_t)( atoi(&line[idx]) );

  /* Read Blue Channel Normalization Range, abort if EOF */
  if( Load_Line(line, glrptrc, "Blue Channel Normalization Range") != SUCCESS )
    return( false );
  rc_data.norm_range[BLUE][NORM_RANGE_BLACK] = (uint8_t)( atoi(line) );
  idx = 0;
  while( line[idx++] != '-' );
  rc_data.norm_range[BLUE][NORM_RANGE_WHITE] = (uint8_t)( atoi(&line[idx]) );

  /* Read Blue Channel min pixel value in pseudo-color image */
  if( Load_Line(line, glrptrc,
        "Blue Channel min pixel value in pseudo-color image") != SUCCESS )
    return( false );
  rc_data.colorize_blue_min = (uint8_t)( atoi(line) );

  /* Read Blue Channel max pixel value to enhance in pseudo-color image */
  if( Load_Line(line, glrptrc,
        "Blue Channel max pixel value in to enhance pseudo-color image") != SUCCESS )
    return( false );
  rc_data.colorize_blue_max = (uint8_t)( atoi(line) );

  /* Read Blue Channel pixel value above which we assume it is a cloudy area */
  if( Load_Line(line, glrptrc,
        "Blue Channel cloud area pixel value threshold") != SUCCESS )
    return( false );
  rc_data.clouds_threshold = (uint8_t)( atoi(line) );

  /* Check low pass filter bandwidth. It should be at
   * least 100kHz and no more than about 200kHz */
  if( (rc_data.sdr_filter_bw < MIN_BANDWIDTH) ||
      (rc_data.sdr_filter_bw > MAX_BANDWIDTH) )
  {
    Show_Message(
        "Invalid Roofing Filter Bandwidth\n"\
          "Quit and correct glrptrc", "red" );
    Error_Dialog();
    return( false );
  }

  /* Set Gain control buttons and slider */
  if( rc_data.tuner_gain > 0.0 )
  {
    GtkWidget *radiobtn =
      Builder_Get_Object( main_window_builder, "manual_agc_radiobutton" );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(radiobtn), true );
    ClearFlag( TUNER_GAIN_AUTO );
  }
  else
  {
    GtkWidget *radiobtn =
      Builder_Get_Object( main_window_builder, "auto_agc_radiobutton" );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(radiobtn), true );
    SetFlag( TUNER_GAIN_AUTO );
  }

  /* Initialize top window etc */
  Initialize_Top_Window();

  fclose( glrptrc );

  return( false );
}

/*****************************************************************************/

/* Find_Config_Files()
 *
 * Searches glrpt's home directory for per-satellite configuration
 * files and sets up the "Select Satellite" menu item accordingly
 */
bool Find_Config_Files(void) {
  char *ext;
  struct dirent **file_list;
  int num_files, idx;
  GtkWidget *sat_menu;


  /* Build "Select Satellite" Menu item */
  if( !popup_menu )
    popup_menu = create_popup_menu( &popup_menu_builder );
  sat_menu = Builder_Get_Object( popup_menu_builder, "select_satellite" );

  /* Look for files with a .cfg extention */
  errno = 0;
  found_cfg = 0;
  num_files = scandir( rc_data.glrpt_cfgs, &file_list, NULL, alphasort );
  for( idx = 0; idx < num_files; idx++ )
  {
    /* Look for files with a ".cfg" extention */
    if( (ext = strstr(file_list[idx]->d_name, ".cfg")) )
    {
      /* Cut off file extention to create a satellite name */
      *ext = '\0';

      /* Append new child items to Select Satellite menu */
      GtkWidget *menu_item =
        gtk_menu_item_new_with_label( file_list[idx]->d_name );
      g_signal_connect( menu_item, "activate",
          G_CALLBACK( on_satellite_menuitem_activate ), NULL );
      gtk_widget_show(menu_item);
      gtk_menu_shell_append( GTK_MENU_SHELL(sat_menu), menu_item );

      /* Make first entry the default */
      if( rc_data.satellite_name[0] == '\0' )
        Strlcpy( rc_data.satellite_name,
            file_list[idx]->d_name, sizeof(rc_data.satellite_name) );

      found_cfg++;
    } /* if( (ext = strstr(file_list[idx]->d_name, ".cfg")) ) */
  } /* for( idx = 0; idx < num_files; idx++ ) */

  /* Check for number of config files found */
  if( !found_cfg )
  {
    Show_Message( "No configuration file(s) found", "red" );
    Error_Dialog();
    return( false );
  }

  return false;
}
