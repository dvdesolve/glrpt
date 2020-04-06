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

#ifndef GLRPT_DISPLAY_H
#define GLRPT_DISPLAY_H

#include "../lrpt_demod/demod.h"

#include <cairo/cairo.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <stdint.h>

/* IFFT Signal Amplitude averaging window */
#define AMPL_AVE_WIN   4
#define AMPL_AVE_MUL   3

/* Parameters used in coloring level bars */
#define TRANSITION_BAND   0.2
#define RED_THRESHOLD     4.0
#define GREEN_THRESHOLD   1.5

void Draw_Level_Gauge(GtkWidget *widget, cairo_t *cr, double level);
void Display_Icon(GtkWidget *img, const gchar *name);
void Display_Demod_Params(Demod_t *demod);
void Display_QPSK_Const(int8_t *buffer);
void Display_Waterfall(void);

#endif
