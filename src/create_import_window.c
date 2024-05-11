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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "gettext.h"
#include "widgets.h"
#include "allocate.h"
#include "show_info.h"
#include "functions.h"
#include "import_functions.h"
#include "create_import_window.h"

extern gchar *GP_PASSWD_BUF;


void create_import_window(struct w *widgets)
{
    /* Create and populate the import window */
    FILE *fp;
    int i;
    long conf_size;
    char *old_buffer, *new_buffer;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gchar *utf8 = NULL;
    gchar *username, *groupname;
    GtkTreeViewColumn *user_col, *group_col;
    GtkCellRenderer *import_cell_renderer;
    GtkWidget *import_vbox;
    GtkWidget *import_label0;
    GtkWidget *import_scrolledwindow;
    GtkWidget *label4;
    GtkWidget *import_hbox1;
    GtkWidget *import_label1;
    GtkWidget *import_hbox3;
    GtkWidget *import_label2;
    GtkWidget *import_hbox2;
    GtkWidget *import_label4;
    GtkWidget *spacer_label1;
    GtkWidget *import_hbuttonbox;
    GtkWidget *import_button;
    GtkWidget *alignment1;
    GtkWidget *hbox2;
    GtkWidget *image1;
    GtkWidget *label2;
    GtkWidget *import_cancel_button;
    GtkWidget *alignment2;
    GtkWidget *hbox3;
    GtkWidget *image2;
    GtkWidget *label3;
    gint active_index = 0;
    gchar *info;

    /* We dont import database users yet */
    active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[8]));
    if( active_index > 1  )
    {
        info = g_strdup_printf(
            _("Importing users from a database is not supported yet.\n"));
        show_info(info);
        g_free(info);
        return;
    }

    widgets->import_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(widgets->import_window, 560, 350);
    gtk_window_set_title(GTK_WINDOW(widgets->import_window), _("Select any users you want to import"));
    gtk_window_set_position(GTK_WINDOW(widgets->import_window), GTK_WIN_POS_CENTER);

    import_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(widgets->import_window), import_vbox);

    import_label0 = gtk_label_new(_("Import users to the selected server."));
    gtk_box_pack_start(GTK_BOX(import_vbox), import_label0, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(import_label0), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding(GTK_MISC(import_label0), 0, 5);

    import_scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(import_vbox), import_scrolledwindow, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(import_scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    widgets->import_treeview = gtk_tree_view_new();
    gtk_container_add(GTK_CONTAINER(import_scrolledwindow), widgets->import_treeview);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(widgets->import_treeview), TRUE);

    widgets->import_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(widgets->import_treeview), GTK_TREE_MODEL(widgets->import_store));

    /* Set the column labels in the treeview */
    import_cell_renderer = gtk_cell_renderer_text_new();

    user_col = gtk_tree_view_column_new_with_attributes(_("Username"), import_cell_renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->import_treeview), GTK_TREE_VIEW_COLUMN(user_col));

    group_col = gtk_tree_view_column_new_with_attributes(_("Groupname"), import_cell_renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->import_treeview), GTK_TREE_VIEW_COLUMN(group_col));


    /* Selection is multiple */
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widgets->import_treeview));
    gtk_tree_selection_set_mode(GTK_TREE_SELECTION(selection), GTK_SELECTION_MULTIPLE);

    spacer_label1 = gtk_label_new(" ");
    gtk_box_pack_start(GTK_BOX(import_vbox), spacer_label1, FALSE, FALSE, 0);

    label4 = gtk_label_new(_("Selected users will be imported with the following settings:"));
    gtk_box_pack_start(GTK_BOX(import_vbox), label4, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label4), GTK_JUSTIFY_LEFT);

    import_hbox1 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(import_vbox), import_hbox1, FALSE, FALSE, 0);

    import_label1 = gtk_label_new(_("FTP home directory:"));
    gtk_box_pack_start(GTK_BOX(import_hbox1), import_label1, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(import_label1), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment(GTK_MISC(import_label1), 0.02, 0.5);
    gtk_misc_set_padding(GTK_MISC(import_label1), 5, 10);

    widgets->import_home_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(import_hbox1), widgets->import_home_entry, TRUE, TRUE, 20);
    gtk_entry_set_text(GTK_ENTRY(widgets->import_home_entry), "/var/ftp");
    // FIX: Get this from settings.conf

    import_hbox3 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(import_vbox), import_hbox3, FALSE, FALSE, 0);

    import_label2 = gtk_label_new(_("Add users to private directories below the FTP home directory:"));
    gtk_box_pack_start(GTK_BOX(import_hbox3), import_label2, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(import_label2), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment(GTK_MISC(import_label2), 0.02, 0.5);
    gtk_misc_set_padding(GTK_MISC(import_label2), 5, 7);

    widgets->import_with_username_checkbutton = gtk_check_button_new_with_mnemonic("");
    gtk_box_pack_start(GTK_BOX(import_hbox3), widgets->import_with_username_checkbutton, FALSE, FALSE, 0);

    import_hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(import_vbox), import_hbox2, FALSE, FALSE, 0);

    import_label4 = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(import_vbox), import_label4, FALSE, FALSE, 0);
    gtk_widget_set_size_request(import_label4, -1, 16);
    gtk_label_set_justify(GTK_LABEL(import_label4), GTK_JUSTIFY_LEFT);

    import_hbuttonbox = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(import_vbox), import_hbuttonbox, FALSE, FALSE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(import_hbuttonbox), GTK_BUTTONBOX_SPREAD);

    import_button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(import_hbuttonbox), import_button);
    //GTK_WIDGET_SET_FLAGS(import_button, GTK_CAN_DEFAULT);
    gtk_widget_set_can_default(import_button, TRUE);

    alignment1 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(import_button), alignment1);

    hbox2 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(alignment1), hbox2);

    image1 = gtk_image_new_from_stock("gtk-yes", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(hbox2), image1, FALSE, FALSE, 0);

    label2 = gtk_label_new_with_mnemonic(_("Import"));
    gtk_box_pack_start(GTK_BOX(hbox2), label2, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label2), GTK_JUSTIFY_LEFT);

    import_cancel_button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(import_hbuttonbox), import_cancel_button);
    //GTK_WIDGET_SET_FLAGS(import_cancel_button, GTK_CAN_DEFAULT);
	gtk_widget_set_can_default(import_cancel_button, TRUE);
	
    alignment2 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(import_cancel_button), alignment2);

    hbox3 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(alignment2), hbox3);

    image2 = gtk_image_new_from_stock("gtk-cancel", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(hbox3), image2, FALSE, FALSE, 0);

    label3 = gtk_label_new_with_mnemonic(_("Cancel"));
    gtk_box_pack_start(GTK_BOX(hbox3), label3, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label3), GTK_JUSTIFY_LEFT);


    g_signal_connect((gpointer) import_button,
            "clicked", G_CALLBACK(import_button_clicked), widgets);

    g_signal_connect_swapped((gpointer) import_cancel_button,
            "clicked", G_CALLBACK(gtk_widget_destroy), G_OBJECT(widgets->import_window));

    gtk_widget_show_all(widgets->import_window);



    /* Populate the import treeview with users */

    if((fp = fopen(GP_PASSWD_BUF, "r")) == NULL)
    {
        printf("Cant open passwd here:\n%s\n", GP_PASSWD_BUF);
        return;
    }

    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    old_buffer = allocate(conf_size);
    new_buffer = allocate(8192);

    while(fgets(old_buffer, conf_size, fp) != NULL)
    {
        if( strlen(old_buffer) > 10 && strlen(old_buffer) < 4000 )
        {
            for(i = 0; old_buffer[i] != '\0'; i++)
                if( old_buffer[i] == ':' )
                    break;

            strcpy(new_buffer, old_buffer);
            new_buffer[i] = '\0';

            /* Dont insert root or the user the server runs as */
            if( ! strcmp(new_buffer, "root") || ! strcmp(new_buffer, SERVER_USER) )
                continue;


            gtk_list_store_append(GTK_LIST_STORE(widgets->import_store), &iter);

            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->import_store), &iter, 0, utf8, -1);

            /* Get this users groupname */
            username = g_strdup_printf("%s", new_buffer);
            groupname = get_user_setting(username, "group");
            snprintf(new_buffer, 4000, "%s", groupname);
            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->import_store), &iter, 1, utf8, -1);
            g_free(username);
            g_free(groupname);
        }
    }
    fclose(fp);

    free(old_buffer);
    free(new_buffer);

    if( utf8 != NULL )
        g_free(utf8);
}
