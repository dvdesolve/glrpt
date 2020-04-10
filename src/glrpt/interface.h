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

#ifndef GLRPT_INTERFACE_H
#define GLRPT_INTERFACE_H

/*****************************************************************************/

#include <glib.h>
#include <gtk/gtk.h>

#include <stddef.h>

/*****************************************************************************/

GtkWidget *Builder_Get_Object(GtkBuilder *builder, gchar *name);
GtkWidget *create_main_window(GtkBuilder **builder);
GtkWidget *create_popup_menu(GtkBuilder **builder);
GtkWidget *create_timer_dialog(GtkBuilder **builder);
GtkWidget *create_error_dialog(GtkBuilder **builder);
GtkWidget *create_startstop_timer(GtkBuilder **builder);
GtkWidget *create_quit_dialog(GtkBuilder **builder);
void Initialize_Top_Window(void);

/*****************************************************************************/

#endif
