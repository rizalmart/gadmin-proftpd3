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


#include "../config.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include "widgets.h"
#include "gettext.h"
#include "show_info.h"
#include "create_file_tab.h"
#include "populate_file_tab.h"
#include "clear_file_tab.h"

/* Wether or not to let the file section expand */
#define EXPAND_FILE_SECTION TRUE

void generate_welcome_stats(struct w *widgets);
void generate_html_stats(struct w *widgets);

/* Generates ftp statistics as welcome.msg for each user */
void generate_welcome_stats(struct w *widgets)
{
    FILE *fp;
    gchar *cmd, *info;

    cmd = g_strdup_printf("gprostats -w %s", WELCOME_MESSAGE);

    if((fp = popen(cmd, "r")) == NULL)
    {
        info = g_strdup_printf(_("Error running command: %s\n"), cmd);
        show_info(info);
        g_free(info);
    }
    else
        pclose(fp);

    g_free(cmd);
}


/* Generates ftp statistics as a html file */
void generate_html_stats(struct w *widgets)
{
    FILE *fp;
    gchar *cmd, *info;

    cmd = g_strdup_printf(_("gprostats -html %s"), HTML_STATISTICS);

    if((fp = popen(cmd, "r")) == NULL)
    {
        info = g_strdup_printf(_("Error running command: %s\n"), cmd);
        show_info(info);
        g_free(info);
    }
    else
        pclose(fp);

    g_free(cmd);
}


void create_file_tab(struct w *widgets)
{
    GtkCellRenderer *file_cell_renderer;
    GtkWidget *file_treeview_hbox;
    GtkWidget *file_scrolled_window;
    gchar *utf8 = NULL;

    /* Create the file treeview in a scrolled window */
    file_treeview_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox5), file_treeview_hbox, EXPAND_FILE_SECTION, TRUE, 0);

    file_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(file_treeview_hbox), file_scrolled_window, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(file_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    widgets->file_store = gtk_list_store_new(6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    widgets->file_treeview = gtk_tree_view_new();
    gtk_tree_view_set_model(GTK_TREE_VIEW(widgets->file_treeview), GTK_TREE_MODEL(widgets->file_store));

    gtk_container_add(GTK_CONTAINER(file_scrolled_window), widgets->file_treeview);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(widgets->file_treeview), TRUE);

    /* Set the column labels in the treeview */
    file_cell_renderer = gtk_cell_renderer_text_new();

    GtkTreeViewColumn *username_col = gtk_tree_view_column_new_with_attributes(_("Username"), file_cell_renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->file_treeview), GTK_TREE_VIEW_COLUMN(username_col));

    GtkTreeViewColumn *action_col = gtk_tree_view_column_new_with_attributes(_("Action"), file_cell_renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->file_treeview), GTK_TREE_VIEW_COLUMN(action_col));

    GtkTreeViewColumn *bytes_col = gtk_tree_view_column_new_with_attributes(_("Bytes"), file_cell_renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->file_treeview), GTK_TREE_VIEW_COLUMN(bytes_col));

    GtkTreeViewColumn *filename_col = gtk_tree_view_column_new_with_attributes(_("Files"), file_cell_renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->file_treeview), GTK_TREE_VIEW_COLUMN(filename_col));

    GtkTreeViewColumn *hostname_col = gtk_tree_view_column_new_with_attributes(_("Client"), file_cell_renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->file_treeview), GTK_TREE_VIEW_COLUMN(hostname_col));

    GtkTreeViewColumn *sot_col = gtk_tree_view_column_new_with_attributes(_("Start of transfer"), file_cell_renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->file_treeview), GTK_TREE_VIEW_COLUMN(sot_col));



    /* The update and clear hbutton box */
    GtkWidget *file_button_box = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox5), file_button_box, FALSE, FALSE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(file_button_box), GTK_BUTTONBOX_SPREAD);

    /* Clear files button */
    GtkWidget *clear_file_button = gtk_button_new_from_stock(GTK_STOCK_DELETE);
    gtk_box_pack_start(GTK_BOX(file_button_box), clear_file_button, FALSE, FALSE, 0);
    g_signal_connect_swapped(G_OBJECT(clear_file_button), "clicked", G_CALLBACK(clear_file_tab), widgets);

    /* Update files button */
    GtkWidget *update_file_button = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
    gtk_box_pack_start(GTK_BOX(file_button_box), update_file_button, FALSE, FALSE, 0);
    g_signal_connect_swapped(G_OBJECT(update_file_button), "clicked", G_CALLBACK(populate_files), widgets);




    /* The generate statistics hbutton box */
    GtkWidget *gen_button_box = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox5), gen_button_box, FALSE, FALSE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(gen_button_box), GTK_BUTTONBOX_SPREAD);

    /* Generate welcome.msg files in all users ftp root directories */
    GtkWidget *genstats_button1 = gtk_button_new();
    gtk_box_pack_start(GTK_BOX(gen_button_box), genstats_button1, FALSE, FALSE, 0);

    GtkWidget *genstats_alignment1 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(genstats_button1), genstats_alignment1);

    GtkWidget *genstats_hbox1 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(genstats_alignment1), genstats_hbox1);

    GtkWidget *genstats_image1 = gtk_image_new_from_stock("gtk-refresh", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(genstats_hbox1), genstats_image1, FALSE, FALSE, 0);

    GtkWidget *genstats_label1 = gtk_label_new_with_mnemonic(_("Generate welcome messages"));
    gtk_box_pack_start(GTK_BOX(genstats_hbox1), genstats_label1, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(genstats_label1), GTK_JUSTIFY_LEFT);
    g_signal_connect_swapped(G_OBJECT(genstats_button1), "clicked", G_CALLBACK(generate_welcome_stats), widgets);


    /* Generate a ftp.htm file as /var/www/html/ftp.htm */
    GtkWidget *genstats_button2 = gtk_button_new();
    gtk_box_pack_start(GTK_BOX(gen_button_box), genstats_button2, FALSE, FALSE, 0);

    GtkWidget *genstats_alignment2 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(genstats_button2), genstats_alignment2);

    GtkWidget *genstats_hbox2 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(genstats_alignment2), genstats_hbox2);

    GtkWidget *genstats_image2 = gtk_image_new_from_stock("gtk-refresh", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(genstats_hbox2), genstats_image2, FALSE, FALSE, 0);

    GtkWidget *genstats_label2 = gtk_label_new_with_mnemonic(_("Generate html statistics"));
    gtk_box_pack_start(GTK_BOX(genstats_hbox2), genstats_label2, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(genstats_label2), GTK_JUSTIFY_LEFT);
    g_signal_connect_swapped(G_OBJECT(genstats_button2), "clicked", G_CALLBACK(generate_html_stats), widgets);



    gtk_widget_show_all(widgets->main_window);

    if( utf8 != NULL )
        g_free(utf8);
}
