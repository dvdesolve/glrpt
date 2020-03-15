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

#include "utils.h"
#include "../common/shared.h"

/*------------------------------------------------------------------*/

/*  File_Name()
 *
 *  Prepare a file name, use date and time if null argument
 */
  void
File_Name( char *file_name, uint32_t chn, const char *ext )
{
  int len; /* String length of file_name */

  /* If file_name is null, use date and time as file name */
  if( strlen(file_name) == 0 )
  {
    /* Variables for reading time (UTC) */
    time_t tp;
    struct tm utc;
    char tim[20];

    /* Prepare file name as UTC date-time. Default path is images/ */
    time( &tp );
    utc = *gmtime( &tp );
    strftime( tim, sizeof(tim), "%d%b%Y-%H%M", &utc );

    /* Combination pseudo-color image */
    if( chn == 3 )
      snprintf( file_name, MAX_FILE_NAME-1,
          "%simages/%s-Combo%s", rc_data.glrpt_dir, tim, ext );
    else /* Channel image */
      snprintf( file_name, MAX_FILE_NAME-1,
          "%simages/%s-Ch%d%s", rc_data.glrpt_dir, tim, chn, ext );
  }
  else /* Remove leading spaces from file_name */
  {
    int idx = 0;
    len = (int)strlen( file_name );
    while( (file_name[idx] == ' ') && (idx < len) )
    {
      file_name++;
      idx++;
    }
  }

} /* End of File_Name() */

/*------------------------------------------------------------------------*/

/* Filename()
 *
 * Finds file name in a file path
 */

  static char *
Filename( char *fpath )
{
  int idx;

  idx = (int)strlen( fpath );
  while( (--idx >= 0) && (fpath[idx] != '/') );
  return( &fpath[++idx] );

} /* Filename() */

/*------------------------------------------------------------------------*/

/*  Usage()
 *
 *  Prints usage information
 */

  void
Usage( void )
{
  fprintf( stderr, "%s\n",
      "Usage: glrpt [-hv]" );

  fprintf( stderr, "%s\n",
      "       -h: Print this usage information and exit");

  fprintf( stderr, "%s\n",
      "       -v: Print version number and exit");

} /* End of Usage() */

/*------------------------------------------------------------------------*/

/*  Show_Message()
 *
 *  Prints a message string in the Text View scroller
 */

  void
Show_Message( const char *mesg, const char *attr )
{
  GtkAdjustment *adjustment;

  static GtkTextIter iter;
  static gboolean first_call = TRUE;

  /* Initialize */
  if( first_call )
  {
    first_call = FALSE;
    gtk_text_buffer_get_iter_at_offset( text_buffer, &iter, 0 );
  }

  /* Print message */
  gtk_text_buffer_insert_with_tags_by_name(
      text_buffer, &iter, mesg, -1, attr, NULL );
  gtk_text_buffer_insert( text_buffer, &iter, "\n", -1 );

  /* Scroll Text View to bottom */
  adjustment = gtk_scrolled_window_get_vadjustment
    ( GTK_SCROLLED_WINDOW(text_scroller) );
  gtk_adjustment_set_value( adjustment,
      gtk_adjustment_get_upper(adjustment) -
      gtk_adjustment_get_page_size(adjustment) );

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );

} /* End of Show_Message() */

/*------------------------------------------------------------------------*/

/***  Memory allocation/freeing utils ***/
void mem_alloc( void **ptr, size_t req )
{
  *ptr = malloc( req );
  if( *ptr == NULL )
  {
    perror( "glrpt: A memory allocation request failed" );
    exit( -1 );
  }
  memset( *ptr, 0, req );
} /* End of void mem_alloc() */

/*------------------------------------------------------------------------*/

void mem_realloc( void **ptr, size_t req )
{
  *ptr = realloc( *ptr, req );
  if( *ptr == NULL )
  {
    perror( "glrpt: A memory allocation request failed" );
    exit( -1 );
  }
} /* End of void mem_realloc() */

/*------------------------------------------------------------------------*/

void free_ptr( void **ptr )
{
  if( *ptr != NULL ) free( *ptr );
  *ptr = NULL;
} /* End of void free_ptr() */

/*------------------------------------------------------------------------*/

/* Open_File()
 *
 * Opens a file, aborts on error
 */

  gboolean
Open_File( FILE **fp, char *fname, const char *mode )
{
  /* Message buffer */
  char mesg[MESG_SIZE];

  /* Open Channel A image file */
  *fp = fopen( fname, mode );
  if( *fp == NULL )
  {
    snprintf( mesg, sizeof(mesg),
        "glrpt: Failed to open file %s", fname );
    perror( mesg );

    snprintf( mesg, sizeof(mesg),
        "Failed to open file\n%s", Filename(fname) );
    Show_Message( mesg, "red" );
    Error_Dialog();
    return( FALSE );
  }

  return( TRUE );
} /* End of Open_File() */

/*------------------------------------------------------------------------*/

/* Save_Image_JPEG()
 *
 * Write an image buffer to file
 */
  void
Save_Image_JPEG(
    char *file_name,
    int width, int height,
    int num_channels,
    const uint8_t *pImage_data,
    compression_params_t *comp_params )
{
  char mesg[MESG_SIZE];
  gboolean ret;

  /* Open image file, abort on error */
  snprintf( mesg, sizeof(mesg),
      "Saving Image: %s", Filename(file_name) );
  Show_Message( mesg, "black" );

  /* Compress image data to jpeg file, report failure */
  ret = jepg_encoder_compress_image_to_file(
      file_name, width, height, num_channels, pImage_data, comp_params );
  if( !ret )
  {
    snprintf( mesg, sizeof(mesg),
        "Failed saving image: %s", Filename(file_name) );
    Show_Message( mesg, "red" );
  }

  return;
} /* Save_Image_JPEG() */

/*------------------------------------------------------------------------*/

/* Save_Image_Raw()
 *
 * Write an image buffer to file
 */

  void
Save_Image_Raw(
    char *fname, const char *type,
    uint32_t width, uint32_t height,
    uint32_t max_val, uint8_t *buffer )
{
  size_t size, fw;
  int ret;
  FILE *fp = 0;
  char mesg[MESG_SIZE];


  /* Open image file, abort on error */
  snprintf( mesg, sizeof(mesg), "Saving Image: %s", Filename(fname) );
  Show_Message( mesg, "black" );
  if( !Open_File(&fp, fname, "w") ) return;

  /* Write header in Ch-A output PPM files */
  ret = fprintf( fp, "%s\n%s\n%d %d\n%d\n", type,
      "# Created by glrpt", width, height, max_val );
  if( ret < 0 )
  {
    fclose( fp );
    perror( "glrpt: Error writing image to file" );
    Show_Message( "Error writing image to file", "red" );
    Error_Dialog();
    return;
  }

  /* P6 type (PPM) files are 3* size in pixels */
  if( strcmp(type, "P6") == 0 )
    size = (size_t)( 3 * width * height );
  else
    size = (size_t)( width * height );

  /* Write image buffer to file, abort on error */
  fw = fwrite( buffer, 1, size, fp );
  if( fw != size )
  {
    fclose( fp );
    perror( "glrpt: Error writing image to file" );
    Show_Message( "Error writing image to file", "red" );
    Error_Dialog();
    return;
  }

  fclose( fp );
  return;

} /* Save_Image_Raw() */

/*------------------------------------------------------------------------*/

/*  Cleanup()
 *
 *  Cleanup before quitting or stopping action
 */

  void
Cleanup( void )
{
  /* Deinitialize and free buffers when safe */
  if( isFlagClear(STATUS_DEMODULATING) &&
      isFlagClear(STATUS_RECEIVING) &&
      isFlagClear(STATUS_SOAPYSDR_INIT) &&
      isFlagClear(STATUS_STREAMING) )
  {
    Deinit_Chebyshev_Filter( &filter_data_i );
    Deinit_Chebyshev_Filter( &filter_data_q );
    Deinit_Ifft();
    Demod_Deinit();

    ClearFlag( STATUS_FLAGS_ALL );
  }

  /* Cancel any alarms */
  alarm( 0 );

} /* Cleanup() */

/*------------------------------------------------------------------------*/

/* Functions for testing and setting/clearing flags */

/* An int variable holding the single-bit flags */
static int Flags = 0;

  int
isFlagSet( int flag )
{
  return( Flags & flag );
}

  int
isFlagClear( int flag )
{
  return( !(Flags & flag) );
}

  void
SetFlag( int flag )
{
  Flags |= flag;
}

  void
ClearFlag( int flag )
{
  Flags &= ~flag;
}

  void
ToggleFlag( int flag )
{
  Flags ^= flag;
}

/*------------------------------------------------------------------*/

/* Strlcpy()
 *
 * Copies n-1 chars from src string into dest string. Unlike other
 * such library fuctions, this makes sure that the dest string is
 * null terminated by copying only n-1 chars to leave room for the
 * terminating char. n would normally be the sizeof(dest) string but
 * copying will not go beyond the terminating null of src string
 */
  void
Strlcpy( char *dest, const char *src, size_t n )
{
  char ch = src[0];
  int idx = 0;

  /* Leave room for terminating null in dest */
  n--;

  /* Copy till terminating null of src or to n-1 */
  while( (ch != '\0') && (n > 0) )
  {
    dest[idx] = src[idx];
    idx++;
    ch = src[idx];
    n--;
  }

  /* Terminate dest string */
  dest[idx] = '\0';

} /* Strlcpy() */

/*------------------------------------------------------------------*/

/* Strlcat()
 *
 * Concatenates at most n-1 chars from src string into dest string.
 * Unlike other such library fuctions, this makes sure that the dest
 * string is null terminated by copying only n-1 chars to leave room
 * for the terminating char. n would normally be the sizeof(dest)
 * string but copying will not go beyond the terminating null of src

 */
  void
Strlcat( char *dest, const char *src, size_t n )
{
  char ch = dest[0];
  int idd = 0; /* dest index */
  int ids = 0; /* src  index */

  /* Find terminating null of dest */
  while( (n > 0) && (ch != '\0') )
  {
    idd++;
    ch = dest[idd];
    n--; /* Count remaining char's in dest */
  }

  /* Copy n-1 chars to leave room for terminating null */
  n--;
  ch = src[ids];
  while( (n > 0) && (ch != '\0') )
  {
    dest[idd] = src[ids];
    ids++;
    ch = src[ids];
    idd++;
    n--;
  }

  /* Terminate dest string */
  dest[idd] = '\0';

} /* Strlcat() */

/*------------------------------------------------------------------*/

/* Clamps a double value between min and max */
  inline double
dClamp( double x, double min, double max )
{
  double ret = x;
  if( x < min ) ret = min;
  else if( x > max ) ret = max;
  return( ret );
}

/*------------------------------------------------------------------*/

/* Clamps an integer value between min and max */
  inline int
iClamp( int i, int min, int max )
{
  int ret = i;
  if( i < min ) ret = min;
  else if( i > max ) ret = max;
  return( ret );
}

/*------------------------------------------------------------------*/

