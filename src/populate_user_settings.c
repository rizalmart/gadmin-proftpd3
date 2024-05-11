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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "gettext.h"
#include "allocate.h"
#include "widgets.h"
#include "functions.h"
#include "populate_user_settings.h"
#include "show_info.h"
#include "populate_shell_combo.h"


extern char global_server_address[1024];
extern char global_server_port[1024];
extern char global_server_name[1024];
extern char global_server_type[1024];
extern char global_user_name[1024];

extern int use_ratio;
extern int use_quota;


/* Move to header */
void set_toggle_values(char *new_buffer, GtkTreeIter iter, struct w *widgets);




/* Set toggle values for each directory in the directory treeview */
void set_toggle_values(char *new_buffer, GtkTreeIter iter, struct w *widgets)
{
    int x = 0;

    if( strstr(new_buffer, "LIST") || strstr(new_buffer, "NLST") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 1, 1, -1);

    if( strstr(new_buffer, "STOR") || strstr(new_buffer, "STOU") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 2, 1, -1);

    if( strstr(new_buffer, "APPE") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 3, 1, -1);

    if( strstr(new_buffer, "RETR") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 4, 1, -1);

    if( strstr(new_buffer, "RNFR") || strstr(new_buffer, "RNTO") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 5, 1, -1);

    if( strstr(new_buffer, "DELE") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 7, 1, -1);

    if( strstr(new_buffer, "MKD") || strstr(new_buffer, "XMKD") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 8, 1, -1);

    if( strstr(new_buffer, "RMD") || strstr(new_buffer, "XRMD") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 9, 1, -1);

    if( strstr(new_buffer, "SITE") )  /* dont match SITE_CHMOD or SITE_CHGRP */
    {
        for(x = 0; new_buffer[x] != '\0'; x++)
        {
            if( new_buffer[x]=='\0' || new_buffer[x+1]=='\0' || new_buffer[x+2]=='\0'
            ||  new_buffer[x+3]=='\0' || new_buffer[x+4]=='\0' )
                break;

            if( new_buffer[x]=='S' && new_buffer[x+1]=='I' && new_buffer[x+2]=='T'
            &&  new_buffer[x+3]=='E' && new_buffer[x+4]==' ' )
            {
                gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 10, 1, -1);
                break;
            }
        }
    }

    if( strstr(new_buffer, "SITE_CHMOD") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 11, 1, -1);

    if( strstr(new_buffer, "SITE_CHGRP") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 12, 1, -1);

    if( strstr(new_buffer, "MTDM") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 13, 1, -1);

    /* Show working directory */
    if( strstr(new_buffer, "WD") )  /* PWD and XPWD */
    {
        for(x = 0; new_buffer[x] != '\0'; x++)
        {
            if( new_buffer[x]=='\0' || new_buffer[x+1]=='\0' || new_buffer[x+2]=='\0' )
                break;

            if( new_buffer[x]=='P'&& new_buffer[x+1]=='W' && new_buffer[x+2]=='D')
            {
                gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 14, 1, -1);
                break;
            }
        }
    }

    if( strstr(new_buffer, "SIZE") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 15, 1, -1);

    if( strstr(new_buffer, "STAT") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 16, 1, -1);

    if( strstr(new_buffer, "CWD") && strstr(new_buffer, "XCWD") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 17, 1, -1);

    if( strstr(new_buffer, "CDUP") && strstr(new_buffer, "XCUP") )
        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 18, 1, -1);
}



void populate_user_settings(struct w *widgets)
{
    FILE *fp;
    gchar *utf8 = NULL;
    gchar *info;
    GtkTreeIter iter;
    char *new_buffer, *line, *user_buffer, *address_buffer;
    char *port_buffer, *spinval;
    char *fr, *frc, *br, *brc;  /* Used for ratio settings */
    long file_size;
    int found = 0, homedir_vals_set = 0, i = 0, x = 0;
    int stats_checked = 1;
    gint spinvalue = 0;
    gchar *username, *what, *val;

    /* Clear the directory treeview */
    gtk_list_store_clear(GTK_LIST_STORE(widgets->directory_store));

    username = g_strdup_printf("%s", global_user_name);

    /* Always set an empty password */
    gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[1]), "");


    /* Set all widget values to default if no user is selected */
    info = g_strdup_printf("%s", global_user_name);
    if( info == NULL || strlen(info) < 1 )
    {
        /* User */
        gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[0]), "");

        /* Password */
        gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[1]), "");

        /* Group */
        gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[2]), "");

        /* Comment */
        gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[3]), "");

        /* Shell */
        gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->user_set_combo[0]), 0);

        /* Require password */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[1]), TRUE);

        /* Show user in statistics */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[2]), TRUE);

        /* Max logins, set to 10 */
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[0]), 10);

        /* Allow logins from */
        gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[5]), "All");

        /* Ratio defaults to disabled */
        if( use_ratio )
        {
            /* Byte ratio */
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[1]), 0);

            /* Byte ratio credit */
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[2]), 0);

            /* File ratio */
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[3]), 0);

            /* File ratio credit */
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[4]), 0);
        }
    }
    g_free(info);


    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        info = g_strdup_printf(_("Error reading the configuration here:\n%s\n"), PROFTPD_CONF);
        show_info(info);
        g_free(info);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);
    new_buffer = allocate(file_size + 1);
    user_buffer = allocate(4096);
    address_buffer = allocate(8192);
    port_buffer = allocate(8192);


    /* Set the banned checkbuttons state */
    if( is_banned(global_user_name) )
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[0]), TRUE);
    else
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[0]), FALSE);


    /* Dont match a similar user */
    snprintf(user_buffer, 4000, "User %s\n", global_user_name);


    if( strstr((char *)global_server_type, "Virtualhost") )
        sprintf(address_buffer, "<VirtualHost %s>\n", global_server_address);
    else
        sprintf(address_buffer, global_server_address);

    sprintf(port_buffer, "Port %s\n", global_server_port);


    /* Scroll to the selected vhost */
    if( strstr((char *)global_server_type, "Virtualhost") )
    {
        if( file_size > 1 )
            while(fgets(line, file_size, fp) != NULL)
            {
                /* If its the correct address */
                if( strcmp(line, address_buffer) == 0 )
                {
                    while(fgets(line, file_size, fp) != NULL)
                    {
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

    /* We have scrolled to a vhost or just began at the top */
    found = 0;

    if( file_size > 1 )
        while(fgets(line, file_size, fp) != NULL)
        {
            /* Pick up all <Anonymous (with the the root directory) */
            if( strstr(line, "<Anonymous") )
                strcpy(new_buffer, line);

            if( strlen(line) > 4000 )
            {
                info = g_strdup_printf(_("A line with over 4000 chars is not valid in:\n%s\n"), PROFTPD_CONF);
                show_info(info);
                fclose(fp);
                free(line);
                free(new_buffer);
                free(user_buffer);
                free(address_buffer);
                free(port_buffer);
                free(info);
                return;
            }

            /* We have found the correct user, insert its settings */
            if( strcmp(line, user_buffer) == 0 && ! strstr(line, "AllowUser")
            &&  ! strstr(line, "DenyUser") && ! strstr(line, "FakeUser") )
            {
                /* Insert the root directory to the directory treeview (collected in the loop above) */
                new_buffer[strlen(new_buffer) - 2] = '\0';
                utf8 = g_locale_to_utf8(&new_buffer[11], strlen(&new_buffer[11]), NULL, NULL, NULL);
                gtk_list_store_append(GTK_LIST_STORE(widgets->directory_store), &iter);
                gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 0, utf8, -1);

                /* Username on the line after <Anonymous /var/ftp/dir> (cant be set before root dir) */
                sscanf(line, "%*s %s", new_buffer);
                utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[0]), utf8);

                found = 1;

                /* Insert all the standard values like user, group etc */
                while(fgets(line, file_size, fp) != NULL)
                {
                    if( strstr(line, "Group") && found )
                    {
                        sscanf(line, "%*s %s", new_buffer);
                        utf8 = g_locale_to_utf8(new_buffer, strlen(new_buffer), NULL, NULL, NULL);
                        gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[2]), utf8);
                    }

                    if( strstr(line, "AnonRequirePassword") && found )
                    {
                        sscanf(line, "%*s %s", new_buffer);

                        if( strstr(new_buffer, "off") )   /* cant use 'on' so this is reversed */
                            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[1]), FALSE);
                        else
                            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[1]), TRUE);
                    }

                    if( strstr(line, "Allow from") && found )
                    {
                        strcpy(new_buffer, line);
                        for(i = 0; new_buffer[i] != '\0'; i++)
                        {
                            if( i >= 2 && new_buffer[i-2]=='m' )
                            {
                                new_buffer[strlen(new_buffer) - 1] = '\0';
                                utf8 = g_locale_to_utf8(&new_buffer[i], strlen(&new_buffer[i]), NULL, NULL, NULL);
                                gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[5]), utf8);
                                break;
                            }
                            if( i > 1000 )
                                break;
                        }
                    }

                    if( strstr(line, "MaxClients") && found )
                    {
                        for(x = 0; line[x] != '\0'; x++)
                        {
                            if( line[x]=='"' )
                                break;
                        }
                        x++;
                        if( line[x] != '\0' )
                            sprintf(new_buffer, "%s", &line[x]);
                        for(x = 0; new_buffer[x] != '\0'; x++)
                        {
                            if( new_buffer[x]=='"' )
                            {
                                new_buffer[x] = '\0';
                                break;
                            }
                        }
                        /* Max clients spinbutton */
                        spinval = allocate(4096);
                        sscanf(line, "%*s %s", spinval);
                        if( chars_are_digits(spinval) )
                        {
                            spinvalue = atoi(spinval);
                            gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[0]), spinvalue);
                        }
                        free(spinval);
                    }


                    /* Ratio Module */
                    if( use_ratio && strstr(line, "UserRatio ") && found )
                    {
                        /* Allocating all at once for clarity */
                        fr  = allocate(4096);
                        frc = allocate(4096);
                        br  = allocate(4096);
                        brc = allocate(4096);

                        /* Get the ratio numbers (file ratio, fileratio credit, byte ratio, byte ratio credit) */
                        sscanf(line, "%*s %*s %s %s %s %s", fr, frc, br, brc);

                        /* Byte ratio spinbutton */
                        if( chars_are_digits(br) )
                        {
                            spinvalue = atoi(br);
                            gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[1]), spinvalue);
                        }
                        free(br);

                        /* Byte ratio credit spinbutton */
                        if( chars_are_digits(brc) )
                        {
                            spinvalue = atoi(brc);
                            gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[2]), spinvalue);
                        }
                        free(brc);

                        /* File ratio spinbutton */
                        if( chars_are_digits(fr) )
                        {
                            spinvalue = atoi(fr);
                            gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[3]), spinvalue);
                        }
                        free(fr);

                        /* File ratio credit spinbutton */
                        if( chars_are_digits(frc) )
                        {
                            spinvalue = atoi(frc);
                            gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->user_set_spinbutton[4]), spinvalue);
                        }
                        free(frc);
                    }


                    /* AllowOverwite setting in the directory treeview for the root directory */
                    if( strstr(line, "AllowOverwrite on") )
                        gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 6, 1, -1);


                    /* Append the Home directory values */
                    if( strstr(line, "<Limit")
                    &&  ! strstr(line, "<Limit LOGIN")
                    &&  homedir_vals_set == 0 )
                    {
                        strcpy(new_buffer, line);
                        while(fgets(line, file_size, fp) != NULL)
                        {
                            if( strstr(line, "Allow") )
                            {
                                homedir_vals_set = 1;
                                set_toggle_values(new_buffer, iter, widgets);
                            }
                            /* Break in any case */
                            break;
                        }
                    }


                    /* Append other directories to the directory treeview */
                    if( strstr(line, "<Directory ") && found )
                    {
                        strcpy(new_buffer, line);
                        for(i = 0; new_buffer[i] != '\0'; i++)
                        {
                            /* "y " */
                            if( i >= 2 && new_buffer[i-2]=='y' )
                            {
                                new_buffer[strlen(new_buffer)-2]='\0';
                                utf8 = g_locale_to_utf8(&new_buffer[i], strlen(&new_buffer[i]), NULL, NULL, NULL);

                                /* Append the directory */
                                gtk_list_store_append(GTK_LIST_STORE(widgets->directory_store), &iter);
                                gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 0, utf8, -1);

                                break;
                            }
                            if( i > 1000 )
                                break;
                        }

                        /* List settings for this directory */
                        while(fgets(line, file_size, fp) != NULL)
                        {
                            if( strstr(line, "</Directory")
                            ||  strstr(line, "</Anonymous")
                            ||  strstr(line, "</Limit") )
                                break;


                            /* AllowOverwite on/off directory treeview checkbutton */
                            if( strstr(line, "AllowOverwrite on") )
                                gtk_list_store_set(GTK_LIST_STORE(widgets->directory_store), &iter, 6, 1, -1);

                            /* Save the allow limit line with the cmds */
                            if( strstr(line, "<Limit") )
                            {
                                snprintf(new_buffer, 1000, "%s", line);

                                while(fgets(line, file_size, fp) != NULL)
                                {

                                    if( strstr(line, "</Directory")
                                    ||  strstr(line, "</Anonymous")
                                    ||  strstr(line, "</Limit") )
                                        break;

                                    if( strstr(line, "Allow") && !strstr(line, "AllowOverwrite") )
                                    {
                                        /* We have a limit allow directive (with cmds) */
                                        set_toggle_values(new_buffer, iter, widgets);
                                        break;
                                    }
                                }
                                /* We can add more limit types here */
                            }
                        }
                    }           /* If the line contained "<Directory" and the user is found end. */


                    if( strstr(line, "#gplockstats") && found )
                        stats_checked = 0;


                    /* The user has been listed .. break */
                    if( strstr(line, "</Anonymous") )
                        break;
                }
            }

            if( found )
                break;
        }
    fclose(fp);
    free(line);
    free(new_buffer);
    free(user_buffer);

    /* Statistics checkbutton */
    if( stats_checked )
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[2]), TRUE);
    else
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->user_set_checkbutton[2]), FALSE);


    /* Get and insert the users comment */
    what = g_strdup_printf("comment");
    val = get_user_setting(username, what);
    gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[3]), val);
    g_free(what);
    g_free(val);


    /* Get the users shell, itll be the selected shell in the combo */
    what = g_strdup_printf("shell");
    val = get_user_setting(username, what);
    g_free(what);

    /* Select the shell we just got in the shell combo box */
    GtkListStore *shell_store;
    shell_store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(widgets->user_set_combo[0])));

    if( shell_store )
    {
        GtkTreeIter shell_iter;
        gchar *shell;

        if( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(shell_store), &shell_iter) )
        {
            do
            {
                gtk_tree_model_get(GTK_TREE_MODEL(shell_store), &shell_iter, 0, &shell, -1);
                if( shell )
                {
                    if( strcmp(val, shell) == 0 )
                    {
                        gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widgets->user_set_combo[0]), &shell_iter);
                        break;
                    }
                    g_free(shell);
                }
            }
            while(gtk_tree_model_iter_next(GTK_TREE_MODEL(shell_store), &shell_iter));
        }
    }
    
    g_free(val);
    g_free(username);

    if( utf8 != NULL )
        g_free(utf8);

    gtk_widget_show_all(widgets->main_window);
}
