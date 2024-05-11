/* GADMIN-PROFTPD - An easy to use GTK+ frontend for the ProFTPD standalone server.
 * Copyright (C) 2001 - 2013 Magnus Loef <magnus-swe@telia.com> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include "widgets.h"
#include "gettext.h"

void close_process_window(struct w *widgets)
{
    gtk_widget_destroy(widgets->process_window);
    widgets->process_window = NULL;
}

/* Creates and shows a process window if not created.
   Appends database process output to it. */
void show_process_window(struct w *widgets, char *line)
{
    gchar *utf8 = NULL;
    GtkTextIter iter;
    GtkTextMark *mark = NULL;
    gchar *mark_name = NULL;
    GtkTextBuffer *text_buffer = NULL;
    GtkWidget *info_vbox;
    GtkWidget *info_viewport, *scrolled_info_window;
    gchar *info;

    if( line == NULL || strlen(line) < 5 )
        return;

    if( ! widgets->process_window )
    {
        widgets->process_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_widget_set_size_request(widgets->process_window, 630, 350);
        gtk_window_set_position(GTK_WINDOW(widgets->process_window), GTK_WIN_POS_CENTER);

        /* Set window information */
        info = g_strdup_printf(_("GADMIN-PROFTPD %s Process"), VERSION);
        gtk_window_set_title(GTK_WINDOW(widgets->process_window), info);
        g_free(info);

        info_vbox = gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(widgets->process_window), info_vbox);

        /* Put a scrolled window in the vbox */
        scrolled_info_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_box_pack_start(GTK_BOX(info_vbox), scrolled_info_window, TRUE, TRUE, 0);

        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_info_window),
                                          GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

        info_viewport = gtk_viewport_new(NULL, NULL);
        gtk_container_add(GTK_CONTAINER(scrolled_info_window), info_viewport);

        widgets->process_textview = gtk_text_view_new();
        gtk_container_add(GTK_CONTAINER(info_viewport), widgets->process_textview);

        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widgets->process_textview), GTK_WRAP_WORD);

        GtkWidget *close_button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
        gtk_box_pack_start(GTK_BOX(info_vbox), close_button, FALSE, TRUE, 0);

        /* Close window buttons */
        g_signal_connect_swapped((gpointer) close_button,
                "clicked", G_CALLBACK(close_process_window), widgets);

        g_signal_connect_swapped(GTK_WINDOW(widgets->process_window),
                "delete_event", G_CALLBACK(close_process_window), widgets);

        gtk_widget_show_all(widgets->process_window);

        while( gtk_events_pending() )
               gtk_main_iteration();
    }


    /* Insert progress text if any */
    utf8 = g_locale_to_utf8(line, strlen(line), NULL, NULL, NULL);
    if( utf8!=NULL )
    {
        /* Get the textbuffer associated with the progress textview */
        text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->process_textview));

        /* Get the end iter from the buffer */
        gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(text_buffer), &iter);

        /* Append data to the textview at the location specified by iter */
        gtk_text_buffer_insert(text_buffer, &iter, utf8, -1);

        /* Get the end iter from the buffer again */
        gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(text_buffer), &iter);

        /* Create a mark at the end of the buffer specified by iter */
        mark = gtk_text_buffer_create_mark(GTK_TEXT_BUFFER(text_buffer), mark_name, &iter, FALSE);

        /* Move the mark to the bottom of the textview then scroll to it */
        gtk_text_view_move_mark_onscreen(GTK_TEXT_VIEW(widgets->process_textview), mark);

        /* Scroll to the mark */
        gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(widgets->process_textview), mark, 0.4, TRUE, 0.0, 0.0);

        /* Delete the mark */
        gtk_text_buffer_delete_mark(GTK_TEXT_BUFFER(text_buffer), mark);

        g_free(utf8);
    }

    while( gtk_events_pending() )
           gtk_main_iteration();
}
