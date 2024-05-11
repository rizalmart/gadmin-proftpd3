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
#include "populate_user_settings.h"

extern char global_user_name[1024];



void user_treeview_row_clicked(GtkTreeView * treeview, GdkEventButton * event, gpointer data)
{
    gchar *user;
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
            gtk_tree_model_get(list_store, &iter, 0, &user, -1);

            sprintf(global_user_name, "%s", (gchar *) user ? user : "None");

            g_free(user);
            gtk_tree_path_free(path);

            populate_user_settings(data);
        }
    }
}
