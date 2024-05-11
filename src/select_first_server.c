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
#include "select_first_server.h"


extern char global_server_address[1024];
extern char global_server_port[1024];
extern char global_server_type[1024];



void select_first_server(struct w *widgets)
{
    GtkTreePath *path;
    GtkTreeIter iter;
    GtkTreeModel *list_store;
    gchar *address = NULL, *port = NULL, *type = NULL;

    path = gtk_tree_path_new_first();

    list_store = gtk_tree_view_get_model(GTK_TREE_VIEW(widgets->server_treeview));

    if( ! gtk_tree_model_get_iter(list_store, &iter, path) )
    {
        gtk_tree_path_free(path);
        return;
    }

    gtk_tree_model_get(list_store, &iter, 0, &address, -1);
    gtk_tree_model_get(list_store, &iter, 1, &port, -1);
    gtk_tree_model_get(list_store, &iter, 3, &type, -1);

    if( address == NULL || address == NULL || type == NULL )
    {
        gtk_tree_path_free(path);
        return;
    }

    sprintf(global_server_address, "%s", (gchar *) address ? address : "None");
    sprintf(global_server_port, "%s", (gchar *) port ? port : "21");
    sprintf(global_server_type, "%s", (gchar *) type ? type : "None");

    g_free(address);
    g_free(port);
    g_free(type);
}
