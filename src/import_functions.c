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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "support.h"
#include "allocate.h"
#include "widgets.h"
#include "functions.h"
#include "commands.h"
#include "show_info.h"
#include "import_functions.h"
#include "system_defines.h"
#include "populate_users.h"
#include "select_first_user.h"
#include "populate_user_settings.h"
#include "populate_conf_tab.h"
#include "reread_conf.h"

#define GP_OSX_PASS_DUMP "/etc/gp_osx_passdump"

extern char global_server_address[1024];
extern char global_server_port[1024];
extern char global_server_type[1024];
extern char global_user_name[1024];

extern int activated;

/* Temporary option */
extern char global_version[1024];

/* Fix: Set at import_button_pressed */
char import_root_dir[8192] = "";
long num_imported = 0;


void add_import_users(GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, struct w *widgets)
{
    FILE *fp;
    long conf_size;
    char *user_settings, *user_check, *old_buffer = NULL, *new_buffer = NULL;
    char *address_buffer, *port_buffer;
    int limit_access = 0, found_server = 0;
    gchar *username, *groupname, *directory;
    gchar *info;

    gtk_tree_model_get(model, iter, 0, &username, -1);
    gtk_tree_model_get(model, iter, 1, &groupname, -1);
    /* local_username = g_locale_from_utf8(username, -1, NULL, NULL, NULL); */

    /* If theres no username */
    if( strlen(username) == 0 )
    {
        printf("The username length was too short (0)\n");
        return;
    }
    if( strlen(groupname) == 0 )
    {
        printf("The groupname length was too short (0)\n");
        return;
    }

    if( username[0]=='0' || username[0]=='1' || username[0]=='2' || username[0]=='3' || username[0]=='4'
    ||  username[0]=='5' || username[0]=='6' || username[0]=='7' || username[0]=='8' || username[0]=='9' )
    {
        printf("Usernames cant have a digit first\n");
        return;
    }

    if( username[0]=='r' && username[1]=='o' && username[2]=='o'
    &&  username[3]=='t' && strlen(username) == 4 )
    {
        printf("Refusing to add user root\n");
        return;
    }

    if( groupname[0]=='r' && groupname[1]=='o' && groupname[2]=='o'
    &&  groupname[3]=='t' && strlen(groupname) == 4 )
    {
        printf("Refusing to add group root\n");
        return;
    }

    if( strstr((char *)username, "<") || strstr((char *)username, ">") )
    {
        printf("A username containing < or > is not allowed.\n");
        return;
    }
    if( strstr((char *)groupname, "<") || strstr((char *)groupname, ">") )
    {
        printf("A username containing < or > is not allowed.\n");
        return;
    }

    /* If the directory field has less then 3 chars we inform that this cant be done */
    if( strlen(import_root_dir) < 6 )
    {
        printf("The minimum directory path length is 5 chars.\n");
        return;
    }

    address_buffer = allocate(8192 + 15);
    port_buffer = allocate(8192 + 3);

    if( strstr((char *)global_server_type, "Virtualhost") )
        sprintf(address_buffer, "<VirtualHost %s>\n", global_server_address);
    else
        sprintf(address_buffer, "%s", global_server_address);

    sprintf(port_buffer, "Port %s\n", global_server_port);


    /* Checks if the user exists in proftpd.conf, if so just shows a popup */
    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        free(address_buffer);
        free(port_buffer);
        return;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    old_buffer = allocate(conf_size);
    user_check = allocate(4096);

    snprintf(user_check, 4000, "%s\n", username);

    /* Standard server selected, start checking users directly */
    if( ! strstr((char *)global_server_type, "Virtualhost") )
        found_server = 1;

    if( conf_size > 1 )
    while(fgets(old_buffer, conf_size, fp) != NULL)
    {
        if( strlen(old_buffer) > 8000 )
        {
            fclose(fp);
            free(old_buffer);
            free(user_check);
            free(address_buffer);
            free(port_buffer);
            return;
        }


        if( strstr("Virtualhost", (char *)global_server_type)
        &&  ! found_server && strcmp(old_buffer, address_buffer) == 0 )
        {
            /* Lets see if its the same port as the selected one */
            /* If this server has the same port its the correct server */
            while(fgets(old_buffer, conf_size, fp) != NULL)
            {
                if( strlen(old_buffer) > 8000 )
                {
                    fclose(fp);
                    free(old_buffer);
                    free(user_check);
                    free(address_buffer);
                    free(port_buffer);
                    return;
                }

                /* This will expect the servers port on the second line ! 
                 * else itll miss some vaules .. */
                if( strstr(old_buffer, "Port")
                &&  strcmp(old_buffer, port_buffer) == 0 )
                {
                    found_server = 1;
                    break;
                }

                if( strstr(old_buffer, "</VirtualHost>") )
                    break;
            }
        }

        /* Continue until we find the selected server */
        if( ! found_server )
            continue;

        if( found_server )
            break;

        /* Dont change AllowFrom... */
        if( strstr(old_buffer, "<Anonymous") )
            break;
    }


    /* Check if the user exists in this server.... */
    if( conf_size > 1 )
    while(fgets(old_buffer, conf_size, fp) != NULL)
    {
        if( strlen(old_buffer) > 8000 )
        {
            fclose(fp);
            free(old_buffer);
            free(user_check);
            free(address_buffer);
            free(port_buffer);
            return;
        }

        /* Does this user exist in this server... */
        if( strcmp(old_buffer, user_check) == 0 && ! strstr(old_buffer, "AllowUser") )
        {
            info = g_strdup_printf(_("The user: \"%s\" was not imported\nbecause it already existed in this server.\n"), username);
            show_info(info);
            g_free(info);
            free(old_buffer);
            free(user_check);
            free(address_buffer);
            free(port_buffer);
            fclose(fp);
            return;
        }

        /* Dont match users in vhosts for the defualt server */
        if( ! strstr((char *)global_server_type, "Virtualhost") )
        {
            if( strstr(old_buffer, "<VirtualHost") )
                break;
        }

        if( strstr(old_buffer, "</VirtualHost>") )
            break;
    }
    free(user_check);
    free(old_buffer);
    fclose(fp);


    /* Make the users ftp home directory and chmod it to 0755 if it doesnt exist */
    if( ! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets->import_with_username_checkbutton)) )
        directory = g_strdup_printf("%s", import_root_dir);
    else
        directory = g_strdup_printf("%s/%s", import_root_dir, username);

    if( ! file_exists((gchar *) directory) )
        make_dir_chmod((gchar *) directory, "0755");

    /* Create the users profile */
    user_settings = allocate(16384);
    strcpy(user_settings, "\n<Anonymous ");
    strcat(user_settings, directory);
    strcat(user_settings, ">\n");
    strcat(user_settings, "User ");
    strcat(user_settings, username);
    strcat(user_settings, "\nGroup ");
    strcat(user_settings, groupname);
    strcat(user_settings, "\n");
    strcat(user_settings, "AnonRequirePassword on\n");
    strcat(user_settings, "MaxClients 3 \"The server is full, hosting %m users\"\n");
    strcat(user_settings, "DisplayLogin welcome.msg\n");
    strcat(user_settings, "DisplayChdir .msg\n");
    strcat(user_settings, "AllowOverwrite off\n");
    strcat(user_settings, "<Limit LOGIN>\n");
    strcat(user_settings, " Allow from all\n");
    strcat(user_settings, " Deny from all\n");
    strcat(user_settings, "</Limit>\n");
    strcat(user_settings, "<Limit RETR LIST NLST MDTM SIZE STAT CWD XCWD PWD XPWD CDUP XCUP>\n");
    strcat(user_settings, " AllowAll\n");
    strcat(user_settings, "</Limit>\n");
    strcat(user_settings, "<Limit DELE APPE STOR STOU SITE_CHMOD SITE_CHGRP RNFR RNTO MKD XMKD RMD XRMD>\n");
    strcat(user_settings, " DenyAll\n");
    strcat(user_settings, "</Limit>\n");
    strcat(user_settings, "</Anonymous>\n");

    g_free(directory);


    /* Add the new user settings to proftpd.conf in the correct server context.
     * Add AllowUser to the selected server. */

    found_server = 0;

    /* Standard server selected, start adding users directly */
    if( ! strstr((char *)global_server_type, "Virtualhost") )
        found_server = 1;


    /* Add AllowUser UserName to proftpd.conf to the right server */
    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        free(address_buffer);
        free(port_buffer);
        free(user_settings);
        return;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    old_buffer = allocate(conf_size);
    /* The new_buffer will probably also contain the user_settings */
    new_buffer = allocate(conf_size + 8192);

    /* Add the new user to the new conf */
    if( conf_size > 1 )
    while(fgets(old_buffer, conf_size, fp) != NULL)
    {
        if( strlen(old_buffer) > 4000 )
        {
            fclose(fp);
            free(old_buffer);
            free(address_buffer);
            free(port_buffer);
            free(new_buffer);
            free(user_settings);
            return;
        }

        strcat(new_buffer, old_buffer);

        if( strstr("Virtualhost", (char *)global_server_type)
        &&  ! found_server && strcmp(old_buffer, address_buffer) == 0 )
        {
            /* Lets see if its the same port as the selected one */
            /* If this server has the same port its the correct server. */
            while(fgets(old_buffer, conf_size, fp) != NULL)
            {
                if( strlen(old_buffer) > 4000 )
                    continue;

                strcat(new_buffer, old_buffer);

                /* This will expect the servers port on the second line ! 
                 * else itll miss some values .. */
                if( strstr(old_buffer, "Port")
                &&  strcmp(old_buffer, port_buffer) == 0 )
                {
                    found_server = 1;
                    break;
                }

                if( strstr(old_buffer, "</Virtualhost>") )
                    break;
            }
        }


        /* Continue until we find the selected server */
        if( ! found_server )
            continue;


        /* Add AllowUSer Username .. to this server only */
        if( strstr(old_buffer, "<Limit LOGIN>")
        &&  found_server && ! limit_access )
        {
            strcat(new_buffer, "  AllowUser ");
            strcat(new_buffer, username);
            strcat(new_buffer, "\n");
            limit_access = 1;   /* just incase so we just change the first occurance */

            /* Add the user after </Limit> */
            while(fgets(old_buffer, conf_size, fp) != NULL)
            {
                strcat(new_buffer, old_buffer);
                if( strstr(old_buffer, "</Limit>")
                &&  limit_access == 1 )
                {
                    /* Only add it once */
                    limit_access = 2;
                    /* The user was added after </Limit> */

                    /* Global */
                    num_imported++;

                    strcat(new_buffer, user_settings);
                }
            }
        }

        /* Add the new user_settings if we have another user (once) */
        if( strstr(old_buffer, "</Anonymous")
        &&  limit_access == 1 )
        {
            /* Only add it once */
            limit_access = 2;
            /* The user was added after the first user */

            /* Global */
            num_imported++;

            strcat(new_buffer, user_settings);
        }
    }
    fclose(fp);
    free(old_buffer);
    free(address_buffer);
    free(port_buffer);
    free(user_settings);

    g_free(username);
    g_free(groupname);

    /* Write the new configuration */
    if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
    {
        free(new_buffer);
        return;
    }

    fputs(new_buffer, fp);
    fclose(fp);
    free(new_buffer);
}


void import_button_clicked(GtkButton * button, struct w *widgets)
{
    /* Add all the marked users to the selected server, 
     * make dirs and allow them to login */
    GtkTreeSelection *selection;
    G_CONST_RETURN gchar *g_home_dir;
    gchar *info = NULL;

    /* Globals */
    num_imported = 0;
    strcpy(import_root_dir, "");

    g_home_dir = gtk_entry_get_text(GTK_ENTRY(widgets->import_home_entry));

    if( strlen(g_home_dir) <= 4000 )
        sprintf(import_root_dir, "%s", g_home_dir);
    else
    {
        info = g_strdup_printf(_("Error: Home directory too long: %s\n"), g_home_dir);
        show_info(info);
        g_free(info);
        return;
    }

    /* Call foreach on the users */
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widgets->import_treeview));

    gtk_tree_selection_selected_foreach(GTK_TREE_SELECTION(selection),
             (GtkTreeSelectionForeachFunc) & add_import_users, widgets);

    /* Update the users, user settings and the conf tab. */
    populate_users(widgets);
    select_first_user(widgets);
    populate_user_settings(widgets);

    populate_conf_tab(widgets);

    gtk_widget_destroy(widgets->import_window);

    info = g_strdup_printf(_("Total number of imported users: %ld\n"), num_imported);
    show_info(info);
    g_free(info);

    reread_conf(widgets);
}
