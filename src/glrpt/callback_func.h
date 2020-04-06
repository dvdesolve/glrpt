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

#ifndef GLRPT_CALLBACK_FUNC_H
#define GLRPT_CALLBACK_FUNC_H

#include <cairo/cairo.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <stdint.h>

void Error_Dialog(void);
gboolean Cancel_Timer(gpointer data);
void Popup_Menu(void);
void Start_Togglebutton_Toggled(GtkToggleButton *togglebutton);
void Start_Receiver_Menuitem_Toggled(GtkCheckMenuItem *menuitem);
void Decode_Images_Menuitem_Toggled(GtkCheckMenuItem *menuitem);
void Alarm_Action(void);
void Decode_Timer_Setup(void);
void Auto_Timer_OK_Clicked(void);
void Hours_Entry(GtkEditable *editable);
void Minutes_Entry(GtkEditable *editable);
void Enter_Center_Freq(uint32_t freq);
void Fft_Drawingarea_Size_Alloc(GtkAllocation *allocation);
void Qpsk_Drawingarea_Size_Alloc(GtkAllocation *allocation);
void Qpsk_Drawingarea_Draw(cairo_t *cr);
void BW_Entry_Activate(GtkEntry *entry);
void Set_Check_Menu_Item(gchar *item_name, gboolean flag);

#endif
