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
#include "support.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "allocate.h"
#include "widgets.h"
#include "functions.h"
#include "populate_users.h"
#include "show_info.h"
#include "commands.h"


extern char global_server_address[1024];
extern char global_server_port[1024];
extern char global_server_name[1024];
extern char global_server_type[1024];
extern char global_user_name[1024];



void populate_users(struct w *widgets)
{
    /* Lists all the users in the userlist */
    FILE *fp;
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeModel *list_store;
    char *old_buffer, *new_buffer, *address_buffer, *port_buffer, *translate;
    long i = 0, conf_size = 0;
    gboolean edit = 0;
    int found_user = 0, found_server = 0;
    gchar *user, *user_comment;
    gchar *utf8 = NULL;

    gtk_list_store_clear(widgets->user_store);

    /* Populate the userlist from proftpd.conf */
    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        /* Dont show a popup */
        printf("Error opening: [%s] for listing users\n", PROFTPD_CONF);
        return;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    old_buffer = allocate(conf_size);
    new_buffer = allocate(conf_size + 8192);
    address_buffer = allocate(8192 + 15);
    port_buffer = allocate(8192 + 3);

    if( strstr((char *)global_server_type, "Virtualhost") )
        sprintf(address_buffer, "<VirtualHost %s>\n", global_server_address);
    else
        sprintf(address_buffer, global_server_address);

    sprintf(port_buffer, "Port %s\n", global_server_port);


    if( strstr((char *)global_server_type, "Virtualhost") )
    {
        if( conf_size > 1 )
        while(fgets(old_buffer, conf_size, fp) != NULL)
        {
            /* If its the correct address */
            if( strcmp(old_buffer, address_buffer) == 0 )
            {
                while(fgets(old_buffer, conf_size, fp) != NULL)
                {
                    /* List only one Vhost */
                    if( strstr(old_buffer, "</VirtualHost>") )
                        break;

                    if( strcmp(old_buffer, port_buffer) == 0 )
                    {
                        found_server = 1;

                        /* Scroll past some stuff to </Limit */
                        while(fgets(old_buffer, conf_size, fp) != NULL)
                        {
                            if( strstr(old_buffer, "</Limit") )
                                break;
                        }

                        if( found_server )
                            break;
                    }
                }
            }

            if( found_server )
                break;
        }
    }


    /* We have found the correct server, list the users in this server */
    if( conf_size > 1 )
    while(fgets(old_buffer, conf_size, fp) != NULL)
    {
        /* break if the selected server is a vhost and the next vhost is found */
        if( strstr((char *)global_server_type, "Virtualhost")
        &&  strstr(old_buffer, "</VirtualHost") )
            break;

        /* break if the selected server is the default server and a vhost is found */
        if( ! strstr((char *)global_server_type, "Virtualhost")
        &&  strstr(old_buffer, "<VirtualHost") )
            break;

        if( strstr(old_buffer, "<Anonymous ") )
        {
            /* Insert directory */
            found_user = 1;

            /* Scroll to the beginning of the directory path */
            for(i = 0; old_buffer[i] != '\0'; i++)
                if( old_buffer[i] != ' ' )
                    break;
            for(i = i; old_buffer[i] != '\0'; i++)
                if( old_buffer[i] == ' ' && old_buffer[i + 1] != ' ' )
                    break;

            snprintf(new_buffer, conf_size, "%s", &old_buffer[i + 1]);

            /* Cut at path end */
            for(i = 0; new_buffer[i] != '\0'; i++)
                if( new_buffer[i] == '>' )
                    break;
            new_buffer[i] = '\0';

            utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
            gtk_list_store_append(GTK_LIST_STORE(widgets->user_store), &iter);
            gtk_list_store_set(GTK_LIST_STORE(widgets->user_store), &iter, 3, utf8, -1);

            while(fgets(old_buffer, conf_size, fp) != NULL)
            {
                /* Insert username, comment and banned status */
                if( strstr(old_buffer, "User ") && ! strstr(old_buffer, "DirFakeUser") )
                {
                    sscanf(old_buffer, "%*s %s", new_buffer);
                    utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                    gtk_list_store_set(GTK_LIST_STORE(widgets->user_store), &iter, 0, utf8, -1);

                    /* Insert user comment */
                    user_comment = get_user_setting(new_buffer, "comment");
                    utf8 = g_locale_to_utf8(user_comment, strlen(user_comment), NULL, NULL, NULL);
                    gtk_list_store_set(GTK_LIST_STORE(widgets->user_store), &iter, 2, utf8, -1);
                    if( user_comment != NULL )
                        g_free(user_comment);


                    translate = allocate(4096);

                    /* Insert banned status */
                    if( is_banned(new_buffer) )
                    {
                        sprintf(translate, _("yes"));
                        utf8 = g_locale_to_utf8(translate, strlen(translate), NULL, NULL, NULL);
                        gtk_list_store_set(GTK_LIST_STORE(widgets->user_store), &iter, 5, utf8, -1);
                    }
                    else
                    {
                        sprintf(translate, _("no"));
                        utf8 = g_locale_to_utf8(translate, strlen(translate), NULL, NULL, NULL);
                        gtk_list_store_set(GTK_LIST_STORE(widgets->user_store), &iter, 5, utf8, -1);
                    }
                    free(translate);

                }
                /* Insert group */
                if( strstr(old_buffer, "Group ") && ! strstr(old_buffer, "DirFakeGroup") )
                {
                    sscanf(old_buffer, "%*s %s", new_buffer);
                    utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                    gtk_list_store_set(GTK_LIST_STORE(widgets->user_store), &iter, 1, utf8, -1);
                }
                /* Insert require password */
                if( strstr(old_buffer, "AnonRequirePassword ") )
                {
                    sscanf(old_buffer, "%*s %s", new_buffer);
                    translate = allocate(4096);

                    if( strstr(new_buffer, "on") )
                    {
                        sprintf(translate, _("yes"));
                        utf8 = g_locale_to_utf8(translate, strlen(translate), NULL, NULL, NULL);
                        gtk_list_store_set(GTK_LIST_STORE(widgets->user_store), &iter, 4, utf8, -1);
                    }
                    else
                    {
                        sprintf(translate, _("no"));
                        utf8 = g_locale_to_utf8(translate, strlen(translate), NULL, NULL, NULL);
                        gtk_list_store_set(GTK_LIST_STORE(widgets->user_store), &iter, 4, utf8, -1);
                    }
                    free(translate);
                }
                /* Insert maximum connections */
                if( strstr(old_buffer, "MaxClients ") )
                {
                    sscanf(old_buffer, "%*s %s", new_buffer);
                    utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                    gtk_list_store_set(GTK_LIST_STORE(widgets->user_store), &iter, 6, utf8, -1);
                }

                if( strstr(old_buffer, "</Anonymous>") )
                    break;
            }
        }
    }
    fclose(fp);

    free(old_buffer);
    free(new_buffer);
    free(address_buffer);
    free(port_buffer);

    if( utf8 != NULL )
        g_free(utf8);

    /* Cant set treepath if there arent any users/rows, then itll crash */
    /* Also unset global_user_name */
    if( ! found_user )
    {
        strcat(global_user_name, "");
        return;
    }

    path = gtk_tree_path_new_first();
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(widgets->user_treeview), path, NULL, edit);

    /* Set this user as globally selected */
    list_store = gtk_tree_view_get_model(GTK_TREE_VIEW(widgets->user_treeview));
    gtk_tree_model_get_iter(list_store, &iter, path);
    gtk_tree_model_get(list_store, &iter, 0, &user, -1);

    sprintf(global_user_name, "%s", (gchar *) user ? user : "None");
    g_free(user);

    gtk_tree_path_free(path);
}
