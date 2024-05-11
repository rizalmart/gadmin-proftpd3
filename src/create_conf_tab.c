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
#include "create_conf_tab.h"
#include "save_conf_tab.h"
#include "standard_conf.h"


/* Wether or not to let the conf section expand */
#define EXPAND_CONF_SECTION TRUE



void create_conf_tab(struct w *widgets)
{
    GtkWidget *conf_textview_hbox;
    GtkWidget *conf_scrolled_window;
    GtkWidget *conf_label1, *conf_alignment1;
    GtkWidget *conf_image1, *conf_hbox1;
    GtkWidget *conf_label2, *conf_alignment2;
    GtkWidget *conf_image2, *conf_hbox2;
    GtkWidget *conf_label3, *conf_alignment3;
    GtkWidget *conf_image3, *conf_hbox3;
    gchar *utf8 = NULL;

    /* Create the conf textview in a scrolled window */
    conf_textview_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox7), conf_textview_hbox, EXPAND_CONF_SECTION, TRUE, 0);

    conf_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(conf_textview_hbox), conf_scrolled_window, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(conf_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    widgets->conf_textview = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(conf_scrolled_window), widgets->conf_textview);

    /* Create a button box */
    GtkWidget *conf_button_box = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox7), conf_button_box, FALSE, FALSE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(conf_button_box), GTK_BUTTONBOX_SPREAD);

    /* Add standard conf button */
    GtkWidget *standard_conf_button = gtk_button_new();
    gtk_box_pack_start(GTK_BOX(conf_button_box), standard_conf_button, FALSE, FALSE, 0);

    conf_alignment1 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(standard_conf_button), conf_alignment1);

    conf_hbox1 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(conf_alignment1), conf_hbox1);

    conf_image1 = gtk_image_new_from_stock("gtk-convert", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(conf_hbox1), conf_image1, FALSE, FALSE, 0);

    conf_label1 = gtk_label_new_with_mnemonic(_("Standard"));
    gtk_box_pack_start(GTK_BOX(conf_hbox1), conf_label1, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(conf_label1), GTK_JUSTIFY_LEFT);
    g_signal_connect_swapped(G_OBJECT(standard_conf_button), "clicked", G_CALLBACK(create_standard_conf_question), widgets);

    /* Restore conf button */
    GtkWidget *restore_conf_button = gtk_button_new();
    gtk_box_pack_start(GTK_BOX(conf_button_box), restore_conf_button, FALSE, FALSE, 0);

    conf_alignment2 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(restore_conf_button), conf_alignment2);

    conf_hbox2 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(conf_alignment2), conf_hbox2);

    conf_image2 = gtk_image_new_from_stock("gtk-convert", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(conf_hbox2), conf_image2, FALSE, FALSE, 0);

    conf_label2 = gtk_label_new_with_mnemonic(_("Restore"));
    gtk_box_pack_start(GTK_BOX(conf_hbox2), conf_label2, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(conf_label2), GTK_JUSTIFY_LEFT);
    g_signal_connect_swapped(G_OBJECT(restore_conf_button), "clicked", G_CALLBACK(restore_configuration), widgets);

    /* Backup conf button */
    GtkWidget *backup_conf_button = gtk_button_new();
    gtk_box_pack_start(GTK_BOX(conf_button_box), backup_conf_button, FALSE, FALSE, 0);

    conf_alignment3 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(backup_conf_button), conf_alignment3);

    conf_hbox3 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(conf_alignment3), conf_hbox3);

    conf_image3 = gtk_image_new_from_stock("gtk-apply", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(conf_hbox3), conf_image3, FALSE, FALSE, 0);

    conf_label3 = gtk_label_new_with_mnemonic(_("Backup"));
    gtk_box_pack_start(GTK_BOX(conf_hbox3), conf_label3, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(conf_label3), GTK_JUSTIFY_LEFT);
    g_signal_connect_swapped(G_OBJECT(backup_conf_button), "clicked", G_CALLBACK(force_configuration_backup), widgets);


    GtkWidget *save_conf_button = gtk_button_new_from_stock(GTK_STOCK_SAVE);
    gtk_box_pack_start(GTK_BOX(conf_button_box), save_conf_button, FALSE, FALSE, 0);
    g_signal_connect_swapped(G_OBJECT(save_conf_button), "clicked", G_CALLBACK(save_conf_tab), widgets);


    gtk_widget_show_all(widgets->main_window);

    if( utf8 != NULL )
        g_free(utf8);
}
