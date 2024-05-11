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
#include "server_treeview_row_clicked.h"
#include "add_server.h"
#include "delete_server.h"
#include "apply_server_settings.h"
#include "create_import_window.h"


/* Wether or not to let the server section expand */
#define EXPAND_SERVER_SECTION FALSE

/* Wether or not to let the server settings section expand */
#define EXPAND_SERVER_SETTINGS_SECTION TRUE



void create_server_tab(struct w *widgets)
{
    GtkCellRenderer *server_cell_renderer;
    GtkWidget *server_treeview_hbox, *settings_treeview_hbox;
    GtkWidget *server_scrolled_window;
    GtkWidget *frame;
    gchar *utf8 = NULL;

    GtkWidget *import_alignment;
    GtkWidget *import_hbox;
    GtkWidget *import_image;
    GtkWidget *import_label;

//    GtkTooltips *tooltips;
//    tooltips = gtk_tooltips_new();


    /* Create the domain treeview in a scrolled window */
    server_treeview_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), server_treeview_hbox, EXPAND_SERVER_SECTION, TRUE, 0);

    server_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(server_treeview_hbox), server_scrolled_window, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(server_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    /* Must set a larger size or it wont scroll */
    gtk_widget_set_size_request(server_scrolled_window, -1, 100);

    widgets->server_store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    widgets->server_treeview = gtk_tree_view_new();
    gtk_tree_view_set_model(GTK_TREE_VIEW(widgets->server_treeview), GTK_TREE_MODEL(widgets->server_store));

    gtk_container_add(GTK_CONTAINER(server_scrolled_window), widgets->server_treeview);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(widgets->server_treeview), TRUE);

    /* Set the column labels in the treeview */
    server_cell_renderer = gtk_cell_renderer_text_new();

    GtkTreeViewColumn *address_col = gtk_tree_view_column_new_with_attributes(_("Server address"), server_cell_renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->server_treeview), GTK_TREE_VIEW_COLUMN(address_col));

    GtkTreeViewColumn *port_col = gtk_tree_view_column_new_with_attributes(_("Port"), server_cell_renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->server_treeview), GTK_TREE_VIEW_COLUMN(port_col));

    GtkTreeViewColumn *server_name_col = gtk_tree_view_column_new_with_attributes(_("Server name"), server_cell_renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->server_treeview), GTK_TREE_VIEW_COLUMN(server_name_col));

    GtkTreeViewColumn *server_type_col = gtk_tree_view_column_new_with_attributes(_("Server type"), server_cell_renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->server_treeview), GTK_TREE_VIEW_COLUMN(server_type_col));

    g_signal_connect((gpointer) widgets->server_treeview, "button_press_event", G_CALLBACK(server_treeview_row_clicked), widgets);


    /* The import, add, delete and change server entries and buttons */
    // Address, port, name(doesnt matter), (type is always vhost)
    GtkWidget *server_button_box = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), server_button_box, FALSE, FALSE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(server_button_box), GTK_BUTTONBOX_SPREAD);


    /* The import users button with its non stock label and image */
    GtkWidget *import_users_button = gtk_button_new();
    gtk_box_pack_start(GTK_BOX(server_button_box), import_users_button, FALSE, FALSE, 0);

    import_alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_widget_show(import_alignment);
    gtk_container_add(GTK_CONTAINER(import_users_button), import_alignment);

    import_hbox = gtk_hbox_new(FALSE, 2);
    gtk_widget_show(import_hbox);
    gtk_container_add(GTK_CONTAINER(import_alignment), import_hbox);

    import_image = gtk_image_new_from_stock("gtk-convert", GTK_ICON_SIZE_BUTTON);
    gtk_widget_show(import_image);
    gtk_box_pack_start(GTK_BOX(import_hbox), import_image, FALSE, FALSE, 0);

    import_label = gtk_label_new_with_mnemonic(_("Import"));
    gtk_widget_show(import_label);
    gtk_box_pack_start(GTK_BOX(import_hbox), import_label, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(import_label), GTK_JUSTIFY_LEFT);
    g_signal_connect_swapped(G_OBJECT(import_users_button), "clicked", G_CALLBACK(create_import_window), widgets);


    /* The delete server button */
    GtkWidget *delete_server_button = gtk_button_new_from_stock(GTK_STOCK_DELETE);
    gtk_box_pack_start(GTK_BOX(server_button_box), delete_server_button, FALSE, FALSE, 0);
    g_signal_connect_swapped(G_OBJECT(delete_server_button), "clicked", G_CALLBACK(delete_server), widgets);

    /* The add server button */
    GtkWidget *add_server_button = gtk_button_new_from_stock(GTK_STOCK_ADD);
    gtk_box_pack_start(GTK_BOX(server_button_box), add_server_button, FALSE, FALSE, 0);
    g_signal_connect_swapped(G_OBJECT(add_server_button), "clicked", G_CALLBACK(add_server), widgets);

    /* The Apply server button */
    GtkWidget *apply_server_button = gtk_button_new_from_stock(GTK_STOCK_APPLY);
    gtk_box_pack_start(GTK_BOX(server_button_box), apply_server_button, FALSE, FALSE, 0);
    g_signal_connect_swapped(G_OBJECT(apply_server_button), "clicked", G_CALLBACK(apply_server_settings), widgets);



    /* Create the server settings in a table inside a scrolled window */
    settings_treeview_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox1), settings_treeview_hbox, EXPAND_SERVER_SETTINGS_SECTION, TRUE, 0);

    widgets->server_settings_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(settings_treeview_hbox), widgets->server_settings_scrolled_window, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widgets->server_settings_scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    /* Must set a larger size or it wont scroll */
    gtk_widget_set_size_request(widgets->server_settings_scrolled_window, -1, 100);

    /* Add a vbox to the scrolled window */
    widgets->server_settings_vbox = gtk_vbox_new(TRUE, 0);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(widgets->server_settings_scrolled_window), widgets->server_settings_vbox);

    /* Add a frame */
    frame = gtk_frame_new(_("Settings"));

    /* Add a table with for the settings with 3 columns */
    widgets->srv_set_table = gtk_table_new(NUM_SERVERTAB_ENTRIES + NUM_SERVERTAB_SPINBUTTONS + NUM_SERVERTAB_COMBOS, 3, FALSE);
    gtk_box_pack_start(GTK_BOX(widgets->server_settings_vbox), frame, TRUE, TRUE, 1);
    gtk_container_add(GTK_CONTAINER(frame), widgets->srv_set_table);


    gtk_widget_show_all(widgets->main_window);

    if( utf8 != NULL )
        g_free(utf8);
}
