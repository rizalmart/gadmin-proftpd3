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
#include "create_transfer_tab.h"
#include "kick_button_clicked.h"
#include "kick_ban_button_clicked.h"

#define EXPAND_TRANSFER_SECTION 1

void transfer_treeview_row_clicked(GtkTreeView * treeview, GdkEventButton * event, struct w *widgets)
{
    gchar *pid, *username;
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeModel *list_store;
    GtkTreeSelection *selection;

    /* The left button is pressed */
    if( ! event->type == GDK_BUTTON_PRESS || ! event->button == 1 )
        return;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

    if( gtk_tree_selection_count_selected_rows(selection) <= 1 )
    {
        /* Get the treepath for the row that was clicked */
        if( gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), event->x, event->y, &path, NULL, NULL, NULL) )
        {
            gtk_tree_selection_unselect_all(selection);
            gtk_tree_selection_select_path(selection, path);

            list_store = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
            gtk_tree_model_get_iter(list_store, &iter, path);
            gtk_tree_model_get(list_store, &iter, 0, &pid, -1);
            gtk_tree_model_get(list_store, &iter, 1, &username, -1);

            /* Insert pid or username */
            if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets->transfer_pid_checkbutton)) )
                gtk_entry_set_text(GTK_ENTRY(widgets->kick_entry), pid);
            else
                gtk_entry_set_text(GTK_ENTRY(widgets->kick_entry), username);

            g_free(pid);
            g_free(username);
            gtk_tree_path_free(path);
        }
    }
}


void create_transfer_tab(struct w *widgets)
{
    GtkCellRenderer *transfer_cell_renderer;
    GtkWidget *transfer_treeview_hbox;
    GtkWidget *transfer_scrolled_window;
    gchar *utf8 = NULL;

//    GtkTooltips *tooltips;
//    tooltips = gtk_tooltips_new();

    /* Create the transfer treeview in a scrolled window */
    transfer_treeview_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox3), transfer_treeview_hbox, EXPAND_TRANSFER_SECTION, TRUE, 0);

    transfer_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(transfer_treeview_hbox), transfer_scrolled_window, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(transfer_scrolled_window),
                                            GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    widgets->transfer_store = gtk_list_store_new(8, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    widgets->transfer_treeview = gtk_tree_view_new();
    gtk_tree_view_set_model(GTK_TREE_VIEW(widgets->transfer_treeview), GTK_TREE_MODEL(widgets->transfer_store));

    gtk_container_add(GTK_CONTAINER(transfer_scrolled_window), widgets->transfer_treeview);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(widgets->transfer_treeview), TRUE);

    /* Set the column labels in the treeview */
    transfer_cell_renderer = gtk_cell_renderer_text_new();

    GtkTreeViewColumn *pid_col = gtk_tree_view_column_new_with_attributes(_("PID"), transfer_cell_renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->transfer_treeview), GTK_TREE_VIEW_COLUMN(pid_col));

    GtkTreeViewColumn *username_col = gtk_tree_view_column_new_with_attributes(_("User"), transfer_cell_renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->transfer_treeview), GTK_TREE_VIEW_COLUMN(username_col));

    GtkTreeViewColumn *action_col = gtk_tree_view_column_new_with_attributes(_("Action"), transfer_cell_renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->transfer_treeview), GTK_TREE_VIEW_COLUMN(action_col));

    GtkTreeViewColumn *speed_col = gtk_tree_view_column_new_with_attributes(_("KiloBytes/sec"), transfer_cell_renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->transfer_treeview), GTK_TREE_VIEW_COLUMN(speed_col));

    GtkTreeViewColumn *dir_col = gtk_tree_view_column_new_with_attributes(_("Directory"), transfer_cell_renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->transfer_treeview), GTK_TREE_VIEW_COLUMN(dir_col));

    GtkTreeViewColumn *file_col = gtk_tree_view_column_new_with_attributes(_("File"), transfer_cell_renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->transfer_treeview), GTK_TREE_VIEW_COLUMN(file_col));

    GtkTreeViewColumn *client_col = gtk_tree_view_column_new_with_attributes(_("Client"), transfer_cell_renderer, "text", 6, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->transfer_treeview), GTK_TREE_VIEW_COLUMN(client_col));

    GtkTreeViewColumn *server_col = gtk_tree_view_column_new_with_attributes(_("Server"), transfer_cell_renderer, "text", 7, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->transfer_treeview), GTK_TREE_VIEW_COLUMN(server_col));

    /* Treeview clicked callback */
    g_signal_connect((gpointer) widgets->transfer_treeview,
            "button_press_event", G_CALLBACK(transfer_treeview_row_clicked), widgets);


    /* Add a speed hbox */
    GtkWidget *speed_hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox3), speed_hbox, FALSE, FALSE, 0);

    /* Transfer speed labels. Text and speeds are added in status_update. */
    widgets->total_bandwidth_label = gtk_label_new("");
    widgets->total_incoming_label = gtk_label_new("");
    widgets->total_outgoing_label = gtk_label_new("");

    gtk_box_pack_start(GTK_BOX(speed_hbox), widgets->total_bandwidth_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(speed_hbox), widgets->total_incoming_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(speed_hbox), widgets->total_outgoing_label, FALSE, FALSE, 0);

    /* Add a kick hbox */
    GtkWidget *kick_hbox1 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox3), kick_hbox1, FALSE, FALSE, 0);

    GtkWidget *kick_entry_label = gtk_label_new("Username or PID to kick or kickban:");
    widgets->kick_entry = gtk_entry_new();

    gtk_box_pack_start(GTK_BOX(kick_hbox1), kick_entry_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(kick_hbox1), widgets->kick_entry, FALSE, FALSE, 0);

    /* Togglebutton for selecting pid instead of user */
    GtkWidget *selection_label = gtk_label_new(_("Select PID:"));
    widgets->transfer_pid_checkbutton = gtk_check_button_new();

    gtk_box_pack_start(GTK_BOX(kick_hbox1), selection_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(kick_hbox1), widgets->transfer_pid_checkbutton, FALSE, FALSE, 0);


    /* Add a kick/kickban button box */
    GtkWidget *transfer_button_box = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox3), transfer_button_box, FALSE, FALSE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(transfer_button_box), GTK_BUTTONBOX_SPREAD);

    /* Add kick/kickban buttons and callbacks */
    GtkWidget *kick_button = gtk_button_new();
    GtkWidget *kick_alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(kick_button), kick_alignment);

    GtkWidget *kick_hbox = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(kick_alignment), kick_hbox);

    GtkWidget *kick_image = gtk_image_new_from_stock("gtk-apply", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(kick_hbox), kick_image, FALSE, FALSE, 0);

    GtkWidget *kick_label = gtk_label_new_with_mnemonic(_("Kick"));
    gtk_box_pack_start(GTK_BOX(kick_hbox), kick_label, FALSE, FALSE, 0);

    gtk_label_set_justify(GTK_LABEL(kick_label), GTK_JUSTIFY_LEFT);

    gtk_box_pack_start(GTK_BOX(transfer_button_box), kick_button, FALSE, FALSE, 0);
    g_signal_connect_swapped(G_OBJECT(kick_button), "clicked", G_CALLBACK(kick_button_clicked), widgets);

    GtkWidget *kickban_button = gtk_button_new();
    GtkWidget *kickban_alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(kickban_button), kickban_alignment);

    GtkWidget *kickban_hbox = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(kickban_alignment), kickban_hbox);

    GtkWidget *kickban_image = gtk_image_new_from_stock("gtk-delete", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(kickban_hbox), kickban_image, FALSE, FALSE, 0);

    GtkWidget *kickban_label = gtk_label_new_with_mnemonic(_("Kickban"));
    gtk_box_pack_start(GTK_BOX(kickban_hbox), kickban_label, FALSE, FALSE, 0);

    gtk_label_set_justify(GTK_LABEL(kickban_label), GTK_JUSTIFY_LEFT);

    gtk_box_pack_start(GTK_BOX(transfer_button_box), kickban_button, FALSE, FALSE, 0);

    g_signal_connect_swapped(G_OBJECT(kickban_button), "clicked", G_CALLBACK(kick_ban_button_clicked), widgets);

    if( utf8 != NULL )
        g_free(utf8);

    gtk_widget_show_all(widgets->main_window);
}
