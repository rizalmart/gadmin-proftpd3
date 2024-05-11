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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "widgets.h"
#include "allocate.h"
#include "commands.h"
#include "functions.h"
#include "kick_ban_button_clicked.h"

extern int activated;

extern int MAX_READ_POPEN;

void kick_ban_button_clicked(struct w *widgets)
{
    /* Ban a user then kick by name or pid */
    FILE *fp;
    char *get_buffer, *kick_name, *kick_pid, *kick;
    int user_kick = 0, pid_kick = 0, was_kicked = 0, was_banned = 0;
    G_CONST_RETURN gchar *username;

    username = gtk_entry_get_text(GTK_ENTRY(widgets->kick_entry));
    if( strlen(username) == 0 )
        return;

    kick_name = allocate(4096);
    kick_pid = allocate(4096);

    if( activated )
    {
        /* Get users pid and name */
        if((fp = popen("ftpwho -v", "r")) == NULL)
            return;

        fflush(fp);

        get_buffer = allocate(MAX_READ_POPEN + 1);
        while(fgets(get_buffer, MAX_READ_POPEN, fp) != NULL)
        {
            kick_pid[0] = '\0';
            kick_name[0] = '\0';
            sscanf(get_buffer, "%s %s", kick_pid, kick_name);

            if( kick_name[strlen(kick_name)-1]=='\n'
            ||  kick_name[strlen(kick_name)-1] == '\r')
                kick_name[strlen(kick_name)-1]='\0';

            /* kill this user by name */
            if( strcmp(username, kick_name) == 0
            &&  chars_are_digits(kick_pid) )
            {
                user_kick = 1;
                break;
            }
            /* Kill this user by pid */
            if( strcmp(username, kick_pid) == 0
            &&  chars_are_digits(kick_pid) )
            {
                pid_kick = 1;
                break;
            }
        }
        pclose(fp);
        free(get_buffer);
    }


    /* Put the user in ftpusers if its not banned already or is a pid */
    if( ! is_banned((char *)username) )
    {
        if((fp = fopen(GP_FTPUSERS, "a")) == NULL)
        {
//      sprintf(info_buffer, _("The user was not banned, could not write to:\n%s"), GP_FTPUSERS);
//      info_window = create_info_window();
//          gtk_widget_show(info_window);
        }
        else
        if( user_kick || pid_kick )
        {
            fputs("\n", fp);
            fputs(kick_name, fp);
            fclose(fp);
            was_banned = 1;
        }
        else 
        if( ! chars_are_digits((char *)username) )
        {
            fputs("\n", fp);
            fputs(username, fp);
            fclose(fp);
            was_banned = 1;
        }
        else
        {
//           strcpy(info_buffer, _("Cant ban a non active user by PID.\n"));
//           strcat(info_buffer, _("Misspelled maybe ?\n"));
//           info_window = create_info_window();
//           gtk_widget_show(info_window);
            fclose(fp);
        }
    }
    else
    if( is_banned((char *)username)
    &&  ! chars_are_digits((char *)username) )
    {
//         strcpy(info_buffer, _("The user was already banned.\n"));
//         info_window = create_info_window();
//         gtk_widget_show(info_window);
    }


    free(kick_name);


    if( user_kick || pid_kick )
    {
        /* Kill the the users pid */
        kick = allocate(8192);
        sprintf(kick, "kill -15 %s", kick_pid);
        if( ! system(kick) )
        {

        }
        free(kick);
        was_kicked = 1;
    }

    /* We want to be notified of additional success or failiures here */
    if( was_banned && ! was_kicked )
    {
//        strcpy(info_buffer, _("The user was not currently in the ftp.\n"));
//  strcat(info_buffer, _("The user was banned but not kicked.\n"));
//  info_window = create_info_window();
//        gtk_widget_show(info_window);
    }
    else
    if( was_banned && was_kicked )
    {
//  strcpy(info_buffer, _("The user was kicked and banned.\n"));
//  info_window = create_info_window();
//        gtk_widget_show(info_window);
    }

    free(kick_pid);
}
