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
#include "../common/shared.h"
#include "interface.h"
#include "../sdr/ifft.h"
#include "../sdr/filters.h"
#include "utils.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*****************************************************************************/

static void sig_handler(int signal);

/*****************************************************************************/

/* main
 * Main program initialization and startup
 */
int main(int argc, char *argv[]) {
    /* New and old actions for sigaction routine */
    struct sigaction sa_new, sa_old;

    /* Initialize new actions */
    sa_new.sa_handler = sig_handler;
    sigemptyset(&sa_new.sa_mask);
    sa_new.sa_flags = 0;

    /* Register function to handle signals */
    sigaction(SIGINT,  &sa_new, &sa_old);
    sigaction(SIGSEGV, &sa_new, 0);
    sigaction(SIGFPE,  &sa_new, 0);
    sigaction(SIGTERM, &sa_new, 0);
    sigaction(SIGABRT, &sa_new, 0);
    sigaction(SIGCONT, &sa_new, 0);
    sigaction(SIGALRM, &sa_new, 0);

    /* Process command line options. Defaults below */
    int option;

    while ((option = getopt(argc, argv, "hv")) != -1)
        switch (option) {
            case 'h': /* Print help and exit */
                Usage();
                exit(0);

                break;

            case 'v': /* Print version info and exit */
                puts(PACKAGE_STRING);
                exit(0);

                break;

            default: /* Print help and exit */
                Usage();
                exit(-1);

                break;
        }

    /* Find and prepare program directories */
    if (!PrepareDirectories()) {
        fprintf(stderr, "glrpt: %s\n", "error during preparing directories");
        exit(-1);
    }

    /* Set path to UI file */
    snprintf(rc_data.glrpt_glade, sizeof(rc_data.glrpt_glade),
            "%s/glrpt.glade", PACKAGE_DATADIR);

    /* Start GTK+ */
    gtk_init(&argc, &argv);

    /* Defaults/initialization */
    rc_data.decode_timer = 0;
    rc_data.ifft_decimate = IFFT_DECIMATE;
    rc_data.satellite_name[0] = '\0';

    /* Create glrpt main window */
    main_window = create_main_window(&main_window_builder);
    gtk_window_set_title(GTK_WINDOW(main_window), PACKAGE_STRING);
    gtk_widget_show(main_window);

    /* Create the text view scroller */
    text_scroller = Builder_Get_Object(main_window_builder, "text_scrolledwindow");

    /* Get text buffer */
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(Builder_Get_Object(
                    main_window_builder, "message_textview")));

    /* Get waterfall and constellation widgets */
    qpsk_drawingarea = Builder_Get_Object(main_window_builder, "qpsk_drawingarea");
    ifft_drawingarea = Builder_Get_Object(main_window_builder, "ifft_drawingarea");

    /* Get "RX status" widgets */
    start_togglebutton    = Builder_Get_Object(main_window_builder, "start_togglebutton");
    pll_ave_entry         = Builder_Get_Object(main_window_builder, "pll_ave_entry");
    pll_freq_entry        = Builder_Get_Object(main_window_builder, "pll_freq_entry");
    pll_lock_icon         = Builder_Get_Object(main_window_builder, "pll_lock_icon");
    agc_gain_entry        = Builder_Get_Object(main_window_builder, "agc_gain_entry");
    sig_level_entry       = Builder_Get_Object(main_window_builder, "sig_level_entry");
    frame_icon            = Builder_Get_Object(main_window_builder, "frame_icon");
    status_icon           = Builder_Get_Object(main_window_builder, "status_icon");
    sig_quality_entry     = Builder_Get_Object(main_window_builder, "sig_quality_entry");
    packet_cnt_entry      = Builder_Get_Object(main_window_builder, "packet_cnt_entry");
    ob_time_entry         = Builder_Get_Object(main_window_builder, "ob_time_entry");
    sig_level_drawingarea = Builder_Get_Object(main_window_builder, "sig_level_drawingarea");
    sig_qual_drawingarea  = Builder_Get_Object(main_window_builder, "sig_qual_drawingarea");
    agc_gain_drawingarea  = Builder_Get_Object(main_window_builder, "agc_gain_drawingarea");
    pll_ave_drawingarea   = Builder_Get_Object(main_window_builder, "pll_ave_drawingarea");

    /* Define some rendering tags */
    gtk_text_buffer_create_tag(text_buffer, "black",
            "foreground", "black", NULL);
    gtk_text_buffer_create_tag(text_buffer, "red",
            "foreground", "red", NULL);
    gtk_text_buffer_create_tag(text_buffer, "orange",
            "foreground", "orange", NULL);
    gtk_text_buffer_create_tag(text_buffer, "green",
            "foreground", "darkgreen", NULL);
    gtk_text_buffer_create_tag(text_buffer, "bold",
            "weight", PANGO_WEIGHT_BOLD, NULL);

    /* Get sizes of displays and initialize */
    GtkAllocation alloc;
    gtk_widget_get_allocation(ifft_drawingarea, &alloc);
    Fft_Drawingarea_Size_Alloc(&alloc);
    gtk_widget_get_allocation(qpsk_drawingarea, &alloc);
    Qpsk_Drawingarea_Size_Alloc(&alloc);

    char ver[32];
    snprintf(ver, sizeof(ver), "Welcome to %s", PACKAGE_STRING);
    Show_Message(ver, "bold");

    /* Find configuration files and open the first as default */
    g_idle_add(Find_Config_Files, NULL);
    g_idle_add(Load_Config, NULL);

    /* Main loop */
    gtk_main();

    return 0;
}

/*****************************************************************************/

/* sig_handler
 *
 * Signal action handler function
 */
static void sig_handler(int signal) {
    if (signal == SIGALRM) {
        Alarm_Action();

        return;
    }

    /* Internal wakeup call */
    if (signal == SIGCONT) return;

    ClearFlag(STATUS_RECEIVING);
    fprintf(stderr, "\n");

    switch (signal) {
        case SIGINT:
            fprintf(stderr, "%s\n", "glrpt: exiting via user interrupt");
            exit(-1);

            break;

        case SIGSEGV:
            fprintf(stderr, "%s\n", "glrpt: segmentation fault");
            exit(-1);

            break;

        case SIGFPE:
            fprintf(stderr, "%s\n", "glrpt: floating point exception");
            exit(-1);

            break;

        case SIGABRT:
            fprintf(stderr, "%s\n", "glrpt: abort signal received");
            exit(-1);

            break;

        case SIGTERM:
            fprintf(stderr, "%s\n", "glrpt: termination request received");
            exit(-1);

            break;
    }
}
