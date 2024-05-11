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
#include "functions.h"
#include "create_user_tab.h"
#include "user_treeview_row_clicked.h"
#include "populate_shell_combo.h"
#include "file_chooser.h"

#define EXPAND_USER_SECTION FALSE
#define EXPAND_USER_SETTINGS_SECTION TRUE


void directory_cell_edited(GtkCellRendererText * cell, gchar * path_string, gchar * new_text, struct w *widgets)
{
    /* Changes the edited directory cell text */
    GtkTreeIter iter;
    GtkTreePath *path = NULL;

    path = gtk_tree_path_new_from_string(path_string);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(widgets->directory_store), &iter, path);

    if( path != NULL )
        gtk_tree_path_free(path);

    gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 0, new_text, -1);
}


void dir_toggle_button_clicked(GtkCellRendererToggle *cell, gchar *path, struct w *widgets)
{
    GtkTreeIter iter;
    gboolean val;
    GtkTreeModel *model;
    GtkTreePath *treepath = NULL;
    int column = (int)g_object_get_data(G_OBJECT(cell), "column");

    treepath = gtk_tree_path_new_from_string(path);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(widgets->directory_store), &iter, treepath);
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(widgets->directory_treeview));

    gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, column, &val, -1);
    if( val )
        gtk_list_store_set(widgets->directory_store, &iter, column, FALSE, -1);
    else
        gtk_list_store_set(widgets->directory_store, &iter, column, TRUE, -1);

    gtk_tree_path_free(treepath);
}


void add_directory(gchar * dir, struct w *widgets)
{
    /* Adds a directory with default permissions to the directory treeview */
    int i = 0;
    GtkTreeIter iter;
    gchar *utf8 = NULL;
    gchar *dir_path;

    dir_path = g_strdup_printf("%s", dir);
    utf8 = g_locale_to_utf8(dir_path, strlen(dir_path), NULL, NULL, NULL);
    gtk_list_store_append(GTK_LIST_STORE(widgets->directory_store), &iter);
    gtk_list_store_set(widgets->directory_store, &iter, 0, utf8, -1);
    g_free(dir_path);

    /* Select a few safe default permissions for the directory */
    for(i = 1; i < 19; i++)
    {
        if( i == 1 || i == 4 || i > 13 )
            gtk_list_store_set(widgets->directory_store, &iter, i, TRUE, -1);
    }

    if( utf8 != NULL )
        g_free(utf8);

   /* The user needs to press apply. Dont do it automatically */
}


void select_directory_clicked(struct w *widgets)
{
    gchar *path = NULL;

    path = get_dialog_path_selection("DIR", "/var/ftp", "None");
    if( path != NULL )
    {
        add_directory(path, widgets);
        g_free(path);
    }
}


void del_directory(struct w *widgets)
{
    /* Deletes the selected directory if its not
       the one listed first (the home/root directory) */
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path;
    gint has_row_above;
    gboolean val;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widgets->directory_treeview));

    val = gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection), &model, &iter);

    path = gtk_tree_model_get_path(GTK_TREE_MODEL(model), &iter);

    has_row_above = gtk_tree_path_prev(path);

    /* A row is selected and its not the first one, remove it */
    //if( val && has_row_above )
        gtk_list_store_remove(widgets->directory_store, &iter);

    gtk_tree_path_free(path);

   /* The user needs to press apply. Dont do it automatically */
}


void create_user_tab(struct w *widgets)
{
    GtkCellRenderer *user_cell_renderer;
    GtkWidget *user_treeview_hbox, *user_settings_treeview_hbox;
    GtkWidget *user_scrolled_window, *directory_scrolled_window;
    GtkCellRenderer *toggle_cell_renderer[19];
    GtkCellRenderer *dir_cell_renderer;
    GtkTreeViewColumn *col[19];
    GtkWidget *frame;
    int i = 0, colnr = 0, cellnr = 0;
    gchar *utf8 = NULL;

    //GtkTooltips *tooltips;
    //tooltips = gtk_tooltips_new();

    /* Create the userlist treeview in a scrolled window */
    user_treeview_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox2), user_treeview_hbox, EXPAND_USER_SECTION, TRUE, 0);

    user_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(user_treeview_hbox), user_scrolled_window, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(user_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    /* Must set a larger size or it wont scroll */
    gtk_widget_set_size_request(user_scrolled_window, -1, 100);

    widgets->user_store = gtk_list_store_new(7,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    widgets->user_treeview = gtk_tree_view_new();
    gtk_tree_view_set_model(GTK_TREE_VIEW(widgets->user_treeview), GTK_TREE_MODEL(widgets->user_store));

    gtk_container_add(GTK_CONTAINER(user_scrolled_window), widgets->user_treeview);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(widgets->user_treeview), TRUE);

    /* Set the column labels in the treeview */
    user_cell_renderer = gtk_cell_renderer_text_new();

    GtkTreeViewColumn *user_col = gtk_tree_view_column_new_with_attributes(
                _("User"), user_cell_renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->user_treeview), GTK_TREE_VIEW_COLUMN(user_col));

    GtkTreeViewColumn *group_col = gtk_tree_view_column_new_with_attributes(
                _("Group"), user_cell_renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->user_treeview), GTK_TREE_VIEW_COLUMN(group_col));

    GtkTreeViewColumn *comment_col = gtk_tree_view_column_new_with_attributes(
                _("Comment"), user_cell_renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->user_treeview), GTK_TREE_VIEW_COLUMN(comment_col));

    GtkTreeViewColumn *homedir_col = gtk_tree_view_column_new_with_attributes(
                _("Home directory"), user_cell_renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->user_treeview), GTK_TREE_VIEW_COLUMN(homedir_col));

    GtkTreeViewColumn *req_password_col = gtk_tree_view_column_new_with_attributes(
                _("Require password"), user_cell_renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->user_treeview), GTK_TREE_VIEW_COLUMN(req_password_col));

    GtkTreeViewColumn *banned_col = gtk_tree_view_column_new_with_attributes(
                _("Banned"), user_cell_renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->user_treeview), GTK_TREE_VIEW_COLUMN(banned_col));

    GtkTreeViewColumn *max_logins_col = gtk_tree_view_column_new_with_attributes(
                _("Maximum connections"), user_cell_renderer, "text", 6, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->user_treeview), GTK_TREE_VIEW_COLUMN(max_logins_col));

    g_signal_connect((gpointer) widgets->user_treeview,
                "button_press_event", G_CALLBACK(user_treeview_row_clicked), widgets);


    /* The delete, new user and apply buttonbox */
    widgets->user_button_box = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox2), widgets->user_button_box, FALSE, FALSE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(widgets->user_button_box), GTK_BUTTONBOX_SPREAD);


    /* Create the user settings scrolled window with a frame and a table */
    user_settings_treeview_hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->notebook_vbox2),
        user_settings_treeview_hbox, EXPAND_USER_SETTINGS_SECTION, TRUE, 0);

    widgets->user_settings_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(user_settings_treeview_hbox),
                widgets->user_settings_scrolled_window, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widgets->user_settings_scrolled_window),
                                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    /* Must set a larger size or it wont scroll */
    gtk_widget_set_size_request(widgets->user_settings_scrolled_window, -1, 100);

    /* Add a vbox to the scrolled window */
    widgets->user_settings_vbox = gtk_vbox_new(FALSE, 0);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(widgets->user_settings_scrolled_window),
                        widgets->user_settings_vbox);

    /* Add a frame */
    frame = gtk_frame_new(_("User settings and directories:"));

    /* A table with NUM settings and 2 columns */
    widgets->usr_set_table =
        gtk_table_new(NUM_USERTAB_ENTRIES + NUM_USERTAB_SPINBUTTONS + NUM_USERTAB_CHECKBUTTONS + NUM_USERTAB_COMBOS, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(widgets->user_settings_vbox), frame, TRUE, TRUE, 1);
    gtk_container_add(GTK_CONTAINER(frame), widgets->usr_set_table);


    /* Create the directory treeview in the user settings scrolled window */
    directory_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(widgets->user_settings_vbox), directory_scrolled_window, FALSE, FALSE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(directory_scrolled_window),
                                            GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    /* Must set a larger size or it wont scroll */
    gtk_widget_set_size_request(directory_scrolled_window, -1, 100);


    /* Directory store with some columns... */
    widgets->directory_store = gtk_list_store_new(19, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
        G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
        G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
        G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);

    /* Create the directory treeeview and add it to the scrolled window */
    widgets->directory_treeview = gtk_tree_view_new();
    gtk_tree_view_set_model(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_MODEL(widgets->directory_store));

    gtk_container_add(GTK_CONTAINER(directory_scrolled_window), widgets->directory_treeview);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(widgets->directory_treeview), TRUE);

    /* Add the directory hbox */
    GtkWidget *add_dir_hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(widgets->user_settings_vbox), add_dir_hbox, FALSE, FALSE, 0);

    /* Add directory button */
    GtkWidget *add_directory_button = gtk_button_new();
    gtk_box_pack_start(GTK_BOX(add_dir_hbox), add_directory_button, FALSE, FALSE, 0);
    //gtk_tooltips_set_tip(tooltips, add_directory_button, _("Add a new directory."), NULL);
    gtk_widget_set_tooltip_text(add_directory_button, _("Add a new directory.")); 
                
    GtkWidget *alignment62 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(add_directory_button), alignment62);

    GtkWidget *hbox60 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(alignment62), hbox60);
    GtkWidget *image52 = gtk_image_new_from_stock("gtk-add", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(hbox60), image52, FALSE, FALSE, 0);

    GtkWidget *label230 = gtk_label_new_with_mnemonic(_("Add directory"));
    gtk_box_pack_start(GTK_BOX(hbox60), label230, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label230), GTK_JUSTIFY_LEFT);

    /* The add directory signal hookup */
    g_signal_connect_swapped(add_directory_button,
                "clicked", G_CALLBACK(select_directory_clicked), widgets);

    /* Spacer label */
    GtkWidget *label910 = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(add_dir_hbox), label910, FALSE, FALSE, 0);
    gtk_widget_set_size_request(label910, 20, -1);
    gtk_label_set_justify(GTK_LABEL(label910), GTK_JUSTIFY_LEFT);

    /* Delete directory button */
    GtkWidget *del_directory_button = gtk_button_new();
    gtk_box_pack_start(GTK_BOX(add_dir_hbox), del_directory_button, FALSE, FALSE, 0);
    //gtk_tooltips_set_tip(tooltips, del_directory_button,
    //            _("Delete the selected directory."), NULL);
    gtk_widget_set_tooltip_text(del_directory_button,
                _("Delete the selected directory."));             

    GtkWidget *alignment64 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(del_directory_button), alignment64);

    GtkWidget *hbox64 = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(alignment64), hbox64);
    GtkWidget *image56 = gtk_image_new_from_stock("gtk-delete", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(hbox64), image56, FALSE, FALSE, 0);

    GtkWidget *label234 = gtk_label_new_with_mnemonic(_("Delete directory"));
    gtk_box_pack_start(GTK_BOX(hbox64), label234, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(label234), GTK_JUSTIFY_LEFT);

    /* The delete directory signal hookup */
    g_signal_connect_swapped(del_directory_button, "clicked", G_CALLBACK(del_directory), widgets);


    /* Spacer label */
    GtkWidget *label340 = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(add_dir_hbox), label340, FALSE, FALSE, 0);
    gtk_widget_set_size_request(label340, 20, -1);
    gtk_label_set_justify(GTK_LABEL(label340), GTK_JUSTIFY_LEFT);


    /* Make the directory cell editable */
    dir_cell_renderer = gtk_cell_renderer_text_new();
    g_object_set(dir_cell_renderer, "editable", TRUE, NULL);
    g_signal_connect(dir_cell_renderer, "edited", G_CALLBACK(directory_cell_edited), widgets);

    /* Add callbacks for the toggle buttons and set data on the cells to column numbers */
    for(i = 0; i < 19; i++)
    {
        toggle_cell_renderer[i] = gtk_cell_renderer_toggle_new();
        g_object_set(toggle_cell_renderer[i], "activatable", TRUE, NULL);

        g_object_set_data(G_OBJECT(toggle_cell_renderer[i]), "column", (void *)(i + 1));

        g_signal_connect(toggle_cell_renderer[i],
                "toggled", G_CALLBACK(dir_toggle_button_clicked), widgets);
    }

    colnr = 0;
    cellnr = 0;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Directory"), dir_cell_renderer, "text", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;                    /* Separate dir cell */

    /* Limit commands, begins at colnr 1 and cells at 0 */
    col[colnr] = gtk_tree_view_column_new_with_attributes(_("List"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Upload"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Append"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Download"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Rename"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Overwrite"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Delete"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Make directory"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Remove directory"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Site"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Chmod"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Change group"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Show dates"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Show working directory"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Show sizes"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Stat"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("Change working directory"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    col[colnr] = gtk_tree_view_column_new_with_attributes(_("CD up"), toggle_cell_renderer[cellnr], "active", colnr, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->directory_treeview), GTK_TREE_VIEW_COLUMN(col[colnr]));
    colnr++;
    cellnr++;

    gtk_widget_show_all(widgets->main_window);

    if( utf8 != NULL )
        g_free(utf8);
}
