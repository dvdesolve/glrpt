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

#include "utils.h"

#include "../common/common.h"
#include "../common/shared.h"
#include "../demodulator/demod.h"
#include "../sdr/filters.h"
#include "../sdr/ifft.h"
#include "callback_func.h"

#include <gtk/gtk.h>
#include <turbojpeg.h>

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/*****************************************************************************/

static bool MkdirRecurse(const char *path);
static const char *Filename(const char *fpath);

/*****************************************************************************/

/* An int variable holding the single-bit flags */
static int Flags = 0;

/*****************************************************************************/

/* PrepareDirectories
 *
 * Find and create (if necessary) dirs with configs and final pictures.
 * XDG specs are supported:
 * https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
 */
bool PrepareDirectories(void) {
    char *var_ptr;

    /* system-wide configs are mandatory */
    snprintf(rc_data.glrpt_cfgs, sizeof(rc_data.glrpt_cfgs),
            "%s/config", PACKAGE_DATADIR);

    /* user-specific configs are optional */
    /*
    if ((var_ptr = getenv("XDG_CONFIG_HOME")))
        snprintf(rc_data.glrpt_ucfgs, sizeof(rc_data.glrpt_ucfgs),
                "%s/%s", var_ptr, PACKAGE_NAME);
    else
        snprintf(rc_data.glrpt_ucfgs,sizeof(rc_data.glrpt_ucfgs),
                "%s/.config/%s", getenv("HOME"), PACKAGE_NAME);

    if (!MkdirRecurse(rc_data.glrpt_ucfgs)) {
        fprintf(stderr, "glrpt: %s\n",
                "can't access/create user config directory");

        rc_data.glrpt_ucfgs[0] = '\0';
    }*/

    /* cache for image storage is mandatory */
    /* TODO allow user to select his own directory */
    if ((var_ptr = getenv("XDG_CACHE_HOME")))
        snprintf(rc_data.glrpt_imgs, sizeof(rc_data.glrpt_imgs),
                "%s/%s", var_ptr, PACKAGE_NAME);
    else
        snprintf(rc_data.glrpt_imgs, sizeof(rc_data.glrpt_imgs),
                "%s/.cache/%s", getenv("HOME"), PACKAGE_NAME);

    if (!MkdirRecurse(rc_data.glrpt_imgs)) {
        fprintf(stderr, "glrpt: %s\n",
                "can't access/create images cache directory");

        return false;
    }

    return true;
}

/*****************************************************************************/

/* MkdirRecurse
 *
 * Create directory and all ancestors.
 * Adapted from:
 * https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950
 */
static bool MkdirRecurse(const char *path) {
    const size_t len = strlen(path);
    char _path[MAX_FILE_NAME];
    char *p;

    /* save directory path in a mutable var */
    if (len > sizeof(_path) - 1) {
        errno = ENAMETOOLONG;
        return false;
    }

    strcpy(_path, path);

    /* walk through the path string */
    for (p = _path + 1; *p; p++) {
        if (*p == '/') {
            /* temporarily truncate path */
            *p = '\0';

            if (mkdir(_path, S_IRWXU) != 0) {
                if (errno != EEXIST)
                    return false;
            }

            /* restore path */
            *p = '/';
        }
    }

    if (mkdir(_path, S_IRWXU) != 0) {
        if (errno != EEXIST)
            return false;
    }

    return true;
}

/*****************************************************************************/

/* File_Name()
 *
 * Prepare a file name, use date and time if null argument
 */
void File_Name(char *file_name, uint32_t chn, const char *ext) {
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

    /* TODO possibly dangerous because of system string length limits */
    /* Combination pseudo-color image */
    if( chn == 3 )
      snprintf( file_name, MAX_FILE_NAME-1,
        "%s/%s-Combo%s", rc_data.glrpt_imgs, tim, ext );
    else /* Channel image */
      snprintf( file_name, MAX_FILE_NAME-1,
        "%s/%s-Ch%u%s", rc_data.glrpt_imgs, tim, chn, ext );
  }
  else /* Remove leading spaces from file_name */
  {
    int idx = 0;
    len = (int)strlen( file_name );
    while( (idx < len) && (file_name[idx] == ' ') )
    {
      file_name++;
      idx++;
    }
  }
}

/*****************************************************************************/

/* Filename()
 *
 * Finds file name in a file path
 * TODO may be it worth to use standard library routine
 */
static const char *Filename(const char *fpath) {
  int idx;

  idx = (int)strlen( fpath );
  while( (--idx >= 0) && (fpath[idx] != '/') );
  return( &fpath[++idx] );
}

/*****************************************************************************/

/* Usage()
 *
 * Prints usage information
 */
void Usage(void) {
  fprintf( stderr, "%s\n",
      "Usage: glrpt [-hv]" );

  fprintf( stderr, "%s\n",
      "       -h: Print this usage information and exit");

  fprintf( stderr, "%s\n",
      "       -v: Print version number and exit");
}

/*****************************************************************************/

/* Show_Message()
 *
 * Prints a message string in the Text View scroller
 */
void Show_Message(const char *mesg, const char *attr) {
  GtkAdjustment *adjustment;

  static GtkTextIter iter;
  static bool first_call = true;

  /* Initialize */
  if( first_call )
  {
    first_call = false;
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
  while( g_main_context_iteration(NULL, false) );
}

/*****************************************************************************/

/*** Memory allocation/freeing utils ***/
void mem_alloc(void **ptr, size_t req) {
  *ptr = malloc( req );
  if( *ptr == NULL )
  {
    perror( "glrpt: A memory allocation request failed" );
    exit( -1 );
  }
  memset( *ptr, 0, req );
}

/*****************************************************************************/

void mem_realloc(void **ptr, size_t req) {
  *ptr = realloc( *ptr, req );
  if( *ptr == NULL )
  {
    perror( "glrpt: A memory allocation request failed" );
    exit( -1 );
  }
}

/*****************************************************************************/

void free_ptr(void **ptr) {
  if( *ptr != NULL ) free( *ptr );
  *ptr = NULL;
}

/*****************************************************************************/

/* Open_File()
 *
 * Opens a file, aborts on error
 */
bool Open_File(FILE **fp, const char *fname, const char *mode) {
    /* Message buffer */
    char mesg[MESG_SIZE];

    *fp = fopen(fname, mode);

    if (*fp == NULL) {
        snprintf(mesg, sizeof(mesg),
                "glrpt: Failed to open file %s", fname);
        perror(mesg);

        snprintf(mesg, sizeof(mesg),
                "Failed to open file\n%s", Filename(fname));
        Show_Message(mesg, "red");
        Error_Dialog();
        return false;
    }

    return true;
}

/*****************************************************************************/

/* Save_Image_JPEG()
 *
 * Write an image buffer to file
 */
void Save_Image_JPEG(
        const char *file_name,
        const int width,
        const int height,
        const bool grayscale,
        const uint8_t *img) {
    char mesg[MESG_SIZE];

    /* Open image file, abort on error */
    snprintf( mesg, sizeof(mesg),
            "Saving Image: %s", Filename(file_name) );
    Show_Message( mesg, "black" );

    /* Compress image data to jpeg file, report failure */
    const int jpeg_pf = (grayscale) ? TJPF_GRAY : TJPF_RGB;
    const int jpeg_subsamp = (grayscale) ? TJSAMP_GRAY : TJSAMP_422;
    const int flags = TJFLAG_ACCURATEDCT;

    unsigned long jpeg_size = tjBufSize(width, height, jpeg_subsamp);
    unsigned char *jpeg_buf = tjAlloc(jpeg_size);

    tjhandle tj_instance = tjInitCompress();

    tjCompress2(tj_instance, img, width, 0, height, jpeg_pf,
            &jpeg_buf, &jpeg_size, jpeg_subsamp, rc_data.jpeg_quality, flags);

    FILE *jpeg_file;

    bool ret = Open_File(&jpeg_file, file_name, "wb");

    if (!ret) {
        snprintf( mesg, sizeof(mesg),
                "Failed saving image: %s", Filename(file_name) );
        Show_Message( mesg, "red" );
    }

    fwrite(jpeg_buf, jpeg_size, 1, jpeg_file);

    fclose(jpeg_file);
    tjFree(jpeg_buf);
    tjDestroy(tj_instance);
}

/*****************************************************************************/

/* Save_Image_Raw()
 *
 * Write an image buffer to file
 */
void Save_Image_Raw(
        char *fname,
        const char *type,
        uint32_t width,
        uint32_t height,
        uint32_t max_val,
        uint8_t *buffer) {
  size_t size, fw;
  int ret;
  FILE *fp = 0;
  char mesg[MESG_SIZE];


  /* Open image file, abort on error */
  snprintf( mesg, sizeof(mesg), "Saving Image: %s", Filename(fname) );
  Show_Message( mesg, "black" );
  if( !Open_File(&fp, fname, "w") ) return;

  /* Write header in Ch-A output PPM files */
  ret = fprintf( fp, "%s\n%s\n%u %u\n%u\n", type,
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
}

/*****************************************************************************/

/* Cleanup()
 *
 * Cleanup before quitting or stopping action
 */
void Cleanup(void) {
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
}

/*****************************************************************************/

/* Functions for testing and setting/clearing flags */

int isFlagSet(int flag) {
  return( Flags & flag );
}

/*****************************************************************************/

int isFlagClear(int flag) {
  return( !(Flags & flag) );
}

/*****************************************************************************/

void SetFlag(int flag) {
  Flags |= flag;
}

/*****************************************************************************/

void ClearFlag(int flag) {
  Flags &= ~flag;
}

/*****************************************************************************/

/* Strlcpy()
 *
 * Copies n-1 chars from src string into dest string. Unlike other
 * such library fuctions, this makes sure that the dest string is
 * null terminated by copying only n-1 chars to leave room for the
 * terminating char. n would normally be the sizeof(dest) string but
 * copying will not go beyond the terminating null of src string
 */
void Strlcpy(char *dest, const char *src, size_t n) {
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
}
