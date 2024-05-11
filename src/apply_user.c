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
#include <stdlib.h>
#include <string.h>
#include "widgets.h"
#include "gettext.h"
#include "functions.h"
#include "reread_conf.h"
#include "show_info.h"
#include "add_user.h"
#include "apply_user.h"
#include "allocate.h"
#include "populate_users.h"
#include "populate_user_settings.h"
#include "populate_conf_tab.h"
#include "dir_treeview_funcs.h"

extern char global_server_address[1024];
extern char global_server_port[1024];
extern char global_server_type[1024];

extern int activated;
extern int use_ratio;
extern int use_quota;

/* Temporary for option deprecations */
extern char global_version[1024];

/* The current directory beeing added and limited */
extern gchar *homedir;

/* The directory checkbox values */
//gchar *dir_val2[19];
extern long num_rows;
extern int row_pos;
char *user_profile2;


/* Check if the user exists in the selected server */
int user_exists_in_selected_server(struct w *widgets, gchar * username)
{
    FILE *fp;
    long file_size = 0;
    char *line, *user_check;
    char *address_buffer, *port_buffer;
    int found_server = 0, standard_server = 0;

    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size);
    address_buffer = allocate(8192 + 15);
    port_buffer = allocate(8192 + 3);
    user_check = allocate(4096);

    snprintf(user_check, 4000, "User %s\n", username);

    if( strstr((char *)global_server_type, "Virtualhost") )
        sprintf(address_buffer, "<VirtualHost %s>\n", global_server_address);
    else
        standard_server = 1;

    sprintf(port_buffer, "Port %s\n", global_server_port);

    /* Scroll to the correct vhost */
    if( ! standard_server && file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        /* The correct server address is found */
        if( ! found_server && strcmp(line, address_buffer) == 0 )
        {
            /* Lets see if its the same port as the selected one */
            while(fgets(line, file_size, fp) != NULL)
            {
                if( strstr(line, "Port") && strcmp(line, port_buffer) == 0 )
                {
                    found_server = 1;
                    break;
                }
                /* End of vhost, break and check the next vhost */
                if( strstr(line, "</VirtualHost>") )
                    break;
            }
        }
        /* This vhost has the correct address and port */
        if( found_server )
            break;
    }

    /* The selected vhost was not found */
    if( ! standard_server && ! found_server )
    {
        fclose(fp);
        free(user_check);
        free(line);
        free(address_buffer);
        free(port_buffer);

        return 0;
    }

    /* We have begun at the top for the standard server or scrolled to the selected vhost. */

    /* Check if the user exists in this selected vhost or standard server */
    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        if( strcmp(line, user_check) == 0
        &&  ! strstr(line, "AllowUser")
        &&  ! strstr(line, "FakeUser")   )
        {
            fclose(fp);
            free(user_check);
            free(line);
            free(address_buffer);
            free(port_buffer);

            return 1;
        }

        /* End the search if we are looking for a standard user 
           and the end of the standard server is found */
        if( standard_server && strstr(line, "<VirtualHost") )
            break;

        /* End the search if we are looking for a vhost user 
           and the end of the vhost is found */
        if( ! standard_server && strstr(line, "</VirtualHost") )
            break;
    }
    fclose(fp);
    free(user_check);
    free(line);
    free(address_buffer);
    free(port_buffer);

    return 0;
}


/* Fix mod: group, comment and shell */
void apply_user(struct w *widgets)
{
    /* Change the selected users configuration. */
    /* First delete the user from the conf then add it */
    FILE *fp;
    long file_size;
    int password_length = 0;
    gchar *info;
    char *line, *new_buffer, *config, *address_buffer, *port_buffer;
    long profile_size = 0;
    int user_changed = 0;
    int found = 0;
    gchar *UserUsername;
    char *temp_user;
    gint active_index = 0;

    G_CONST_RETURN gchar *username;
    G_CONST_RETURN gchar *groupname;
    G_CONST_RETURN gchar *comment;
    G_CONST_RETURN gchar *password;
    G_CONST_RETURN gchar *login_from;
    G_CONST_RETURN gchar *max_logins;
    G_CONST_RETURN gchar *br = NULL, *brc = NULL, *fr = NULL, *frc = NULL;

    /* Get the settings from the entries */
    username   = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_entry[0]));
    password   = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_entry[1]));
    groupname  = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_entry[2]));
    comment    = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_entry[3]));
    login_from = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_entry[5]));
    max_logins = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_spinbutton[0]));

    /* Ratios */
    if( use_ratio )
    {
        br = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_spinbutton[1]));
        brc = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_spinbutton[2]));
        fr = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_spinbutton[3]));
        frc = gtk_entry_get_text(GTK_ENTRY(widgets->user_set_spinbutton[4]));
    }

    if( password != NULL )
        password_length = strlen(password);

    /* If the user has written a password thats less then MIN_PASS_LEN chars long */
    if( password_length > 0 && password_length < MIN_PASS_LEN )
    {
        info = g_strdup_printf(_("The minimum password length is %d.\n"), MIN_PASS_LEN);
        show_info(info);
        g_free(info);
        return;
    }

    /* Set global num_rows */
    num_rows = 0;
    gtk_tree_model_foreach(GTK_TREE_MODEL(widgets->directory_store), (GtkTreeModelForeachFunc) num_rows_func, widgets);
    /* There must be atleast one directory */
    if( num_rows < 1 )
    {
        info = g_strdup_printf(_("Missing ftp home directory. Scroll down and add one first.\n"));
        show_info(info);
        g_free(info);
        return;
    }

    if( ! user_exists_in_selected_server(widgets, (gchar *)username) )
    {
        /* Add a new user */
        add_user(widgets);
        return;
    }


    /* (Global char) Statics + entries + number of rows (dirlen + APPE STOR STOU etc) */
    profile_size = 16384 + 1400 + (num_rows * 16384);
    /* Allocate the user profile */
    user_profile2 = allocate(profile_size);


    /* Set the users home dir and its values globally */
    row_pos = 0; /* Only get the home directory */
    gtk_tree_model_foreach(GTK_TREE_MODEL(widgets->directory_store), (GtkTreeModelForeachFunc) dirs_foreach, widgets);

    /* Create the user profile... */

    /* Home directory */
    snprintf(user_profile2, 16384, "\n<Anonymous %s>\n", homedir);

    /* Username */
    strcat(user_profile2, "User ");
    strcat(user_profile2, username);
    strcat(user_profile2, "\n");

    /* Groupname */
    strcat(user_profile2, "Group ");
    strcat(user_profile2, groupname);
    strcat(user_profile2, "\n");

    /* Require password */
    if( ! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[1])) )
        strcat(user_profile2, "AnonRequirePassword off\n");
    else
        strcat(user_profile2, "AnonRequirePassword on\n");

    /* Maximum logins */
    strcat(user_profile2, "MaxClients ");
    strcat(user_profile2, max_logins);
    strcat(user_profile2, " \"");
    strcat(user_profile2, "The server is full, hosting %m users");
    strcat(user_profile2, "\"\n");

    /* Static Welcome and chdir msgs */
    strcat(user_profile2, "DisplayLogin welcome.msg\n");
    strcat(user_profile2, "DisplayChdir .msg\n");

    /* Ratio Module (reversed br/fr order in the gui) */
    if( use_ratio )
    {
        /* Ratios have been turned on in the global settings */
        active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[16]));
        if( active_index == 0 )
        {
            strcat(user_profile2, "UserRatio ");
            strcat(user_profile2, username);
            strcat(user_profile2, " ");
            strcat(user_profile2, fr);
            strcat(user_profile2, " ");
            strcat(user_profile2, frc);
            strcat(user_profile2, " ");
            strcat(user_profile2, br);
            strcat(user_profile2, " ");
            strcat(user_profile2, brc);
            strcat(user_profile2, "\n");
        }
    }

    /* Logins from, if nothing is specified we allow everything */
    if( login_from == NULL || strlen(login_from) < 3 )
    {
        strcat(user_profile2, "<Limit LOGIN>\n");
        strcat(user_profile2, "Allow from all\nDeny from all\n");
        strcat(user_profile2, "</Limit>\n");
    }
    else
    {
        strcat(user_profile2, "<Limit LOGIN>\n");
        strcat(user_profile2, "Allow from ");
        strcat(user_profile2, login_from);
        strcat(user_profile2, "\nDeny from all\n");
        strcat(user_profile2, "</Limit>\n");
    }


    /* Append the Limit CMDs for the home directory */
    append_limit_cmds();

    /* Append all other directories to the user profile */
    row_pos = 1;
    gtk_tree_model_foreach(GTK_TREE_MODEL(widgets->directory_store), (GtkTreeModelForeachFunc) dirs_foreach, widgets);


    /* If not checked, dont show this user in the statistics (picked up by gprostats) */
    if( ! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[2])) )
        strcat(user_profile2, "#gplockstats\n");

    /* Profile end */
    strcat(user_profile2, "</Anonymous>\n");

    if( homedir != NULL )
        g_free(homedir);

    /* We have the new profile, insert it where the old user was located in the selected server */
    UserUsername = g_strdup_printf("User %s\n", username);

    address_buffer = allocate(8192 + 15);
    port_buffer = allocate(8192 + 3);

    if( strstr((char *)global_server_type, "Virtualhost") )
        sprintf(address_buffer, "<VirtualHost %s>\n", global_server_address);
    else
        sprintf(address_buffer, global_server_address);

    sprintf(port_buffer, "Port %s\n", global_server_port);


    /* Make changes for this users settings */
    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        free(address_buffer);
        free(port_buffer);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line   = allocate(file_size + 3);
    config = allocate(file_size + profile_size + 3);

    /* If the selected server is a vhost we collect everything up to
       the first vhost with the same name as the selected one */
    if( strstr((char *)global_server_type, "Virtualhost") )
    {
        if( file_size > 1 )
        while(fgets(line, file_size, fp) != NULL)
        {
            strcat(config, line);

            /* If its the correct address */
            if( strcmp(line, address_buffer) == 0 )
            {
                while(fgets(line, file_size, fp) != NULL)
                {
                    strcat(config, line);

                    /* This server was not the right one */
                    if( strstr(line, "</VirtualHost>") )
                        break;

                    if( strcmp(line, port_buffer) == 0 )
                    {
                        found = 1;
                        break;
                    }
                }
            }
            if( found )
                break;
        }
    }


    /* We have scrolled to the correct
       VirtualHost or its the default server */

    new_buffer = allocate(8192);

    /* Change the selected users settings */
    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        if( strstr(line, "<Anonymous ") )
        {
            /* Save this directory incase its not the correct user */
            strcat(new_buffer, line);

            while(fgets(line, file_size, fp) != NULL)
            {
                /* This is the correct user in the correct server */
                if( strstr(line, UserUsername)
                && ! strstr(line, "AllowUser")
                && ! strstr(line, "DirFakeUser")
                && ! user_changed )
                {
                    /* The new profile is added */
                    strcat(config, user_profile2);
                    new_buffer[0] = '\0';
                    user_changed = 1;

                    /* The old profile is deleted */
                    while(fgets(line, file_size, fp) != NULL)
                    {
                        if( strstr(line, "</Anonymous") )
                            break;

                        /* Some more safety breaks */
                        if( strstr(line, "<Anonymous") )
                            break;

                        if( strstr(line, "</VirtualHost") )
                            break;

                        if( strstr(line, "<VirtualHost") )
                            break;
                    }
                }
                else
                {
                    if( strlen(new_buffer) > 0 )
                    {
                        strcat(config, new_buffer);
                        new_buffer[0] = '\0';
                    }
                    strcat(config, line);
                    break;
                }
            }
        }
        else
            strcat(config, line);
    }
    free(line);
    free(new_buffer);
    g_free(UserUsername);
    fclose(fp);

    free(address_buffer);
    free(port_buffer);
    free(user_profile2);

    if( ! user_changed )
    {
        info = g_strdup_printf(_("Error: user not found, couldnt change its settings.\n"));
        show_info(info);
        g_free(info);
        free(config);
        return;
    }

    /* Write the new config with the changed users settings */
    if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
    {
        free(config);
        return;
    }
    fputs(config, fp);
    fclose(fp);
    free(config);


    /* Ban or unban the user in ftpusers depending on the selection */
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[0])) )
    {
        /* Ban the user */
        if( ! is_banned((char *)username) )
        {
            if((fp = fopen(GP_FTPUSERS, "a")) == NULL)
            {
            }
            else
            {
                fputs("\n", fp);
                fputs(username, fp);
                fputs("\n", fp);
                fclose(fp);
            }
        }
    }
    else
    {
        /* Remove the user from ftpusers if its there */
        if( is_banned((char *)username) )
        {
            if((fp = fopen(GP_FTPUSERS, "r")) == NULL)
            {
            }
            else
            {
                fseek(fp, 0, SEEK_END);
                file_size = ftell(fp);
                rewind(fp);

                config = allocate(file_size + 4096);
                line = allocate(file_size + 4096);
                temp_user = allocate(8192);

                if(file_size > 1)
                while(fgets(line, file_size, fp) != NULL)
                {
                    sscanf(line, "%s", temp_user);
                    if( strcmp(temp_user, username) == 0 )
                    {
                        /* Remove it */
                    }
                    else
                        strcat(config, line);
                }
                fclose(fp);
                free(line);
                free(temp_user);

                if((fp = fopen(GP_FTPUSERS, "w+")) == NULL)
                {
                }
                else
                {
                    fputs(config, fp);
                    fclose(fp);
                }
                free(config);
            }
        }
    }


    /* Change the users password if its >= MIN_PASS_LEN */
    if( password_length >= MIN_PASS_LEN )
    {
        /* Auth types */
        active_index = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->server_set_combo[8]));
        if( active_index == 0 )
        {
            /* System users */

#ifndef USE_DARWIN
            if( user_exists(username) )
#elif USE_DARWIN
            if( niutil_user_exists(username) )
#endif
            {
#ifndef USE_DARWIN
                password_user(username, password);
#elif USE_DARWIN
                niutil_password_user(username, password);
#endif
                info = g_strdup_printf(_("\nThe Password was changed.\n"));
                show_info(info);
                g_free(info);
            }
            else
            {
                info = g_strdup_printf(
                _("The password was not changed.\nThe user didnt exist in shadow or passwd.\n"));
                show_info(info);
                g_free(info);
            }
        }
        else
        if( active_index == 1 )
        {
            /* Virtual users */
            if( user_exists(username) )
            {
                password_virtual_user(username, password);
            }
            else
                printf("Virtual user doesnt exist: %s\n", username);
        }
        else
        if( active_index == 2 )
        {
            /* MySQL users */

            /* The MySQL user information is
               added, or changed if it exists */
            add_user(widgets);
        }
        else
        if( active_index == 3 )
        {
            /* LDAP users */
            printf("Selection out of range.\n");
            return;
        }
    }

    fix_newlines_in_conf();

    /* Update the userlist */
    populate_users(widgets);

    /* Populate the user settings (it also populates the directories) */
    populate_user_settings(widgets);

    /* Populate the conf tab */
    populate_conf_tab(widgets);

    /* Update the server */
    reread_conf(widgets);
}
