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

#include "medet.h"

#include "../common/common.h"
#include "../common/shared.h"
#include "correlator.h"
#include "../glrpt/display.h"
#include "../glrpt/utils.h"
#include "met_jpg.h"
#include "met_packet.h"
#include "met_to_data.h"

#include <glib.h>

#include <stdint.h>
#include <stdio.h>

static int ok_cnt, total_cnt;

/*------------------------------------------------------------------------*/

  void
Medet_Init( void )
{
  int idx;

  /* Initialize things */
  Init_Correlator_Tables();
  Mj_Init();
  Mtd_Init( &mtd_record );

  /* Channel_image[idx] is free'd and set to NULL if
   * already allocated, otherwise it is only set to NULL */
  for( idx = 0; idx < CHANNEL_IMAGE_NUM; idx++ )
    free_ptr( (void **)&channel_image[idx] );
  channel_image_size  = 0;
  channel_image_width = METEOR_IMAGE_WIDTH;

  ok_cnt    = 0;
  total_cnt = 1;
}

/*------------------------------------------------------------------------*/

/* Medet_Deinit()
 *
 * My addition, de-inits the met decoder (free's buffer pointers)
 */
  void
Medet_Deinit( void )
{
  free_ptr( (void **)&(mtd_record.v.pair_distances) );
  free_ptr( (void **)&ac_table );
  uint8_t **dec = ret_decoded();
  free_ptr( (void **)dec );
}

/*------------------------------------------------------------------------*/

/* Decode_Image()
 *
 * Decodes images from soft symbols supplied by the demodulator
 */
  void
Decode_Image( uint8_t *in_buffer, int buf_len )
{
  gboolean ok;
  gchar txt[16];

  while( mtd_record.pos < buf_len )
  {
    ok = Mtd_One_Frame( &mtd_record, in_buffer );
    if( ok )
    {
      Parse_Cvcdu( mtd_record.ecced_data, HARD_FRAME_LEN - 132 );
      ok_cnt++;

      if( isFlagClear(FRAME_OK_ICON) )
      {
        Display_Icon( frame_icon, "gtk-yes" );
        SetFlag( FRAME_OK_ICON );
      }
    }
    else
    {
      if( isFlagSet(FRAME_OK_ICON) )
      {
        Display_Icon( frame_icon, "gtk-no" );
        ClearFlag( FRAME_OK_ICON );
      }
    }

    total_cnt++;
  } /* while( mtd_rec.pos < len ) */

  /* Print decoder status data */
  snprintf( txt, sizeof(txt), "%d", mtd_record.sig_q );
  gtk_entry_set_text( GTK_ENTRY(sig_quality_entry), txt );
  int percent = ( 100 * ok_cnt ) / total_cnt;
  snprintf( txt, sizeof(txt), "%d:%d%%", ok_cnt, percent );
  gtk_entry_set_text( GTK_ENTRY(packet_cnt_entry), txt );

} /* Decode_Image() */

/*------------------------------------------------------------------------*/

/* Sig_Quality()
 *
 * Returns the signal quality in the range 0.0-1.0 for the gauge
 */
  double
Sig_Quality( void )
{
  double ret = (double)mtd_record.sig_q / SIG_QUAL_RANGE;
  ret = dClamp( ret, 0.0, 1.0 );
  return( ret );
}
