/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 3 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for( more details:
 *
 *  http://www.gnu.org/copyleft/gpl.txt
 */

#include "met_packet.h"

#include "../common/shared.h"
#include "met_jpg.h"

#include <glib.h>
#include <gtk/gtk.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static gboolean partial_packet = FALSE;
static int last_frame = 0;

/*------------------------------------------------------------------------*/

  static void
Parse_70( uint8_t *p )
{
  int h, m, s;
  gchar txt[12];

  h  = p[8];
  m  = p[9];
  s  = p[10];

  /* Display the Satellite's onboard time */
  snprintf( txt, sizeof(txt), "%02d:%02d:%02d", h, m, s );
  gtk_entry_set_text( GTK_ENTRY(ob_time_entry), txt );
}

/*------------------------------------------------------------------------*/

  static void
Act_Apd( uint8_t *p, uint32_t apid, int pck_cnt )
{
  int mcu_id, q;

  mcu_id   = p[0];
  q = p[5];

  Mj_Dec_Mcus( &p[6], apid, pck_cnt, mcu_id, (uint8_t)q );
}

/*------------------------------------------------------------------------*/

  static void
Parse_Apd( uint8_t *p )
{
  uint16_t w;
  int pck_cnt;
  uint32_t apid;

  w = (uint16_t)p[0];
  w <<= 8;
  w |= (uint16_t)p[1];

  apid = w & 0x07FF;

  pck_cnt = p[2];
  pck_cnt <<= 8;
  pck_cnt |= p[3];
  pck_cnt &= 0x3FFF;

  if( apid == 70 )
    Parse_70( &p[14] );
  else
    Act_Apd( &p[14], apid, pck_cnt );
}

/*------------------------------------------------------------------------*/

  static int
Parse_Partial( uint8_t *p, int len )
{
  int len_pck;

  if( len < 6 )
  {
    partial_packet = TRUE;
    return( 0 );
  }

  len_pck = ( p[4] << 8 ) | p[5];
  if( len_pck >= len - 6 )
  {
    partial_packet = TRUE;
    return( 0 );
  }

  Parse_Apd( p );

  partial_packet = FALSE;
  return( len_pck + 6 + 1 );
}

/*------------------------------------------------------------------------*/

  void
Parse_Cvcdu( uint8_t *p, int len )
{
  int n, data_len, off;
  int ver, fid;
  int frame_cnt;
  uint16_t hdr_off;
  uint16_t w;

  static uint8_t packet_buf[2048];
  static int packet_off = 0;

  w = (uint16_t)( (p[0] << 8) | p[1] );
  ver = w >> 14;
  fid = w & 0x3F;

  frame_cnt = ( p[2] << 16 ) | ( p[3] << 8 ) | p[4];

  w = (uint16_t)( (p[8] << 8) | p[9] );
  hdr_off  = w & 0x07FF;

  if( (ver == 0) | (fid == 0) ) return; //Empty packet

  data_len = len - 10;
  if( frame_cnt == last_frame + 1 )
  {
    if( partial_packet )
    {
      if( hdr_off == PACKET_FULL_MARK ) //Packet could be larger than one frame
      {
        hdr_off = (uint16_t)( len - 10 );
        memmove( &packet_buf[packet_off], &p[10], hdr_off );
        packet_off += hdr_off;
      }
      else
      {
        memmove( &packet_buf[packet_off], &p[10], hdr_off );
        Parse_Partial( packet_buf, packet_off + hdr_off );
      }
    }
  }
  else
  {
    if( hdr_off == PACKET_FULL_MARK ) //Packet could be larger than one frame
      return;
    partial_packet = FALSE;
    packet_off = 0;
  }
  last_frame = frame_cnt;

  data_len -= hdr_off;
  off = hdr_off;
  while( data_len > 0 )
  {
    n = Parse_Partial( &p[10 + off], data_len );
    if( partial_packet )
    {
      packet_off = data_len;
      memmove( packet_buf, &p[10 + off], (size_t)packet_off );
      break;
    }
    else
    {
      off += n;
      data_len -= n;
    }
  }
}
