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
#include <dirent.h>
#include "functions.h"
#include "commands.h"
#include "status_update.h"
#include "show_info.h"

extern int activated;
extern int MAX_READ_POPEN;


char * get_process_pid(char process[1024], char extmatch[1024])
{
    FILE *fp;
    long num, file_size = 4000;
    char *line, *sub_proc_path, *pid;
    int x = 0;
    struct dirent **namelist;

    pid = allocate(1024);
    strcpy(pid, "0");

    sub_proc_path = allocate(1024);

    num = scandir(PROC_PATH, &namelist, 0, alphasort);
    if( num < 0 )
    {
        perror("scandir");
        return pid;
    }
    else
    {
        /* List all directories in PROC_PATH */
        for(x = 0; x < num; x++)
        {
            /* Now list PROC_PATH/24207/cmdline */
            snprintf(sub_proc_path, 1000, "%s/%s/cmdline", PROC_PATH, namelist[x]->d_name);

            if((fp = fopen(sub_proc_path, "r")) == NULL)
            {
                free(namelist[x]);
                continue;
            }
            line = allocate(file_size + 1);

            if( file_size > 1 )
            while(fgets(line, file_size, fp) != NULL)
            {
                /* If the following strings are detected in this file its running */
                if(strstr(line, process) && strstr(line, extmatch))
                {
                    snprintf(pid, 1000, "%s", namelist[x]->d_name);
                    break;
                }
            }
            fclose(fp);
            free(line);
            free(namelist[x]);
        }
    }
    free(namelist);
    free(sub_proc_path);

    return pid;
}


int status_update(struct w *widgets)
{
    /* Status update for ftpwho, total transfer rates and activated/deactivated status. */
    FILE *fp;
    GtkTreeIter iter;
    long i = 0, in_val = 0, out_val = 0, inxfer = 0, outxfer = 0, totalxfer = 0;
    int incoming = 0, outgoing = 0;
    char *line, *tmp;
    gchar *info, *path, *cmd, *utf8 = NULL;
    char *pid;
    GdkColor color;

    pid = get_process_pid("proftpd", "accepting connections");

    if( strcmp(pid, "0") == 0 )
    {
        activated = 0;
        info = g_strdup_printf(_("Status: Deactivated"));
        utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
        gtk_label_set_text(GTK_LABEL(widgets->status_label), utf8);
        g_free(info);
        if( utf8 != NULL )
            g_free(utf8);
        /* Set status color */
        gdk_color_parse("red", &color);
        gtk_widget_modify_fg(widgets->status_label, GTK_STATE_NORMAL, &color);

        /* Show non ftp usage */
        info = g_strdup_printf(_("The server is deactivated.\n"));
        utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);

        if( info != NULL )
            g_free(info);
        if( utf8 != NULL )
            g_free(utf8);

        free(pid);

        return (TRUE);
    }
    else
    {
        activated = 1;
        /* printf("Proftpd is running as pid: %s\n", pid); */
        info = g_strdup_printf(_("Status: Activated"));
        utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
        gtk_label_set_text(GTK_LABEL(widgets->status_label), utf8);
        g_free(info);
        if( utf8 != NULL )
            g_free(utf8);
        /* Set status color */
        gdk_color_parse("dark green", &color);
        gtk_widget_modify_fg(widgets->status_label, GTK_STATE_NORMAL, &color);
    }

    free(pid);

    /* The server is activated, insert ftpwho information in the treeview */

    /* Add the scoreboard file if its missing to avoid missing messages */
    path = g_strdup_printf("%s/%s", GP_VARDIR, "run/proftpd");
    if( file_exists(path) )
    {
        g_free(path);

        path = g_strdup_printf("%s/%s", GP_VARDIR, "run/proftpd/proftpd.scoreboard");
        if( ! file_exists(path) )
        {
            cmd = g_strdup_printf("echo \"NOTHING\" > %s/%s", GP_VARDIR, "run/proftpd/proftpd.scoreboard");
            run_command(cmd);
            g_free(cmd);
        }
    }
    g_free(path);

    /* Again in another common location */
    path = g_strdup_printf("%s/%s", GP_VARDIR, "proftpd");
    if( file_exists(path) )
    {
        g_free(path);

        path = g_strdup_printf("%s/%s", GP_VARDIR, "proftpd/proftpd.scoreboard");
        if( ! file_exists(path) )
        {
            cmd = g_strdup_printf("echo \"NOTHING\" > %s/%s", GP_VARDIR, "proftpd/proftpd.scoreboard");
            run_command(cmd);
            g_free(cmd);
        }
    }
    g_free(path);


    /* FTP who update and total xferrates */
    gtk_list_store_clear(GTK_LIST_STORE(widgets->transfer_store));

/*
standalone FTP daemon [26990], up for 2 days,  4 hrs 26 min
no users connected

standalone FTP daemon [26990], up for 2 days,  4 hrs 25 min
9832 user1    [ 0m15s]  0m10s idle
client: 127.0.0.1 [127.0.0.1]
server: ::ffff:127.0.0.1:21 (0.0.0.0)
protocol: ftp
location: /
Service class                      -   1 user

standalone FTP daemon [26990], up for 2 days,  4 hrs 45 min
9916 user1    [ 0m14s] (n/a) STOR dok_settings.tar.gz
KB/s: 282.00
client: 127.0.0.1 [127.0.0.1]
server: ::ffff:127.0.0.1:21 (0.0.0.0)
protocol: ftp
location: /
Service class                      -   1 user
*/

    cmd = g_strdup_printf("%s -v 2>&1", FTPWHO_BINARY);
    if((fp = popen(cmd, "r")) == NULL)
    {
        return (TRUE);
    }
    g_free(cmd);
    fflush(fp);

    line = allocate(MAX_READ_POPEN + 1);
    tmp = allocate(MAX_READ_POPEN + 20000); /* +20000 for translations and filenames */

    while(fgets(line, MAX_READ_POPEN, fp) != NULL)
    {
        /* Missing ftpwho command */
        if( strstr(line, ": ftpwho: ") )
        {
            info = g_strconcat(
                _("Missing ftpwho command. Need to install proftpd-utils ?\n"),
            NULL);
            utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);
            gtk_list_store_append(GTK_LIST_STORE(widgets->transfer_store), &iter);
            gtk_list_store_set(GTK_LIST_STORE(widgets->transfer_store), &iter, 1, utf8, -1);
            if( info != NULL )
                g_free(info);
            if( utf8 != NULL )
                g_free(utf8);
            break;
        }

        if( ! strstr(line, " idle")  /* idle is located at the end of the line */
        &&  ! strstr(line, " STOR ")
        &&  ! strstr(line, " STOU ")
        &&  ! strstr(line, " APPE ")
        &&  ! strstr(line, " DELE ")
        &&  ! strstr(line, " RETR ") )
            continue;

        gtk_list_store_append(GTK_LIST_STORE(widgets->transfer_store), &iter);

        /* Insert PID */
        sscanf(line, "%s", tmp);
        utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
        gtk_list_store_set(GTK_LIST_STORE(widgets->transfer_store), &iter, 0, utf8, -1);
        if( utf8 != NULL )
            g_free(utf8);

        /* Insert Username */
        sscanf(line, "%*s %s", tmp);
        utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
        gtk_list_store_set(GTK_LIST_STORE(widgets->transfer_store), &iter, 1, utf8, -1);
        if( utf8 != NULL )
            g_free(utf8);

        /* Insert Action */
        if( strstr(line, " idle") )
            snprintf(tmp, 100, "%s", _("Idle"));
        if( strstr(line, " APPE ") || strstr(line, " STOR ") || strstr(line, " STOU ") )
            snprintf(tmp, 100, "%s", _("Uploading"));
        if( strstr(line, " RETR ") )
            snprintf(tmp, 100, "%s", _("Downloading"));
        if( strstr(line, " DELE ") )
            snprintf(tmp, 100, "%s", _("Deleting"));

        utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
        gtk_list_store_set(GTK_LIST_STORE(widgets->transfer_store), &iter, 2, utf8, -1);
        if( utf8 != NULL )
            g_free(utf8);

        /* Insert filename and handle split filenames */
        if( ! strstr(line, " idle") )
            for(i = 0; line[i] != '\0'; i++)
            {
                if( line[i]=='A' && line[i+1]=='P' && line[i+2]=='P' && line[i+3]=='E' )
                {
                    snprintf(tmp, 16384, "%s", &line[i+5]);
                    break;
                }
                if( line[i]=='S' && line[i+1]=='T' && line[i+2]=='O' && line[i+3]=='R' )
                {
                    snprintf(tmp, 16384, "%s", &line[i+5]);
                    break;
                }
                if( line[i]=='S' && line[i+1]=='T' && line[i+2]=='O' && line[i+3]=='U' )
                {
                    snprintf(tmp, 16384, "%s", &line[i+5]);
                    break;
                }
                if( line[i]=='R' && line[i+1]=='E' && line[i+2]=='T' && line[i+3]=='R')
                {
                    snprintf(tmp, 16384, "%s", &line[i+5]);
                    break;
                }
                if( line[i]=='D' && line[i+1]=='E' && line[i+2]=='L' && line[i+3]=='E')
                {
                    snprintf(tmp, 16384, "%s", &line[i+5]);
                    break;
                }
            }
        if( strstr(line, " idle") )
            snprintf(tmp, 16384, "%s", "None");

        if( tmp[strlen(tmp)-1]=='\n' )
            tmp[strlen(tmp)-1]='\0';

        /* Insert action */
        utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
        gtk_list_store_set(GTK_LIST_STORE(widgets->transfer_store), &iter, 5, utf8, -1);
        if( utf8 != NULL )
            g_free(utf8);

        /* Determine if this is an incoming transfer */
        if( strstr(line, " STOR ") || strstr(line, " STOU ") || strstr(line, " APPE ") )
            incoming = 1;
        else
            incoming = 0;

        /* Determine if this is an outgoing transfer */
        if( strstr(line, " RETR ") )
            outgoing = 1;
        else
            outgoing = 0;

        /* Total Xferrates incoming/outgoing/total bandwidth usage */

        /* Scroll to the next line */
        while(fgets(line, MAX_READ_POPEN, fp) != NULL)
            break;

        /* Calculate incoming transfers */
        if( strstr(line, "KB/s: ") && incoming == 1
        &&  ! strstr(line, "inf")  && ! strstr(line, "nan") )
        {
            if( chars_are_digits(&line[7]) )
            {
                sscanf(line, "%*s %li", &in_val);
            }

            if( in_val > 0 )
            {
                inxfer = inxfer + in_val;
                totalxfer = totalxfer + in_val;
            }
        }
        /* Calculate outgoing transfers */
        if( strstr(line, "KB/s: ") && outgoing == 1
        &&  ! strstr(line, "inf") && ! strstr(line, "nan") )
        {
            if( chars_are_digits(&line[7]) )
                sscanf(line, "%*s %li", &out_val);

            if( out_val > 0 )
            {
                outxfer = outxfer + out_val;
                totalxfer = totalxfer + out_val;
            }
        }

        /* Insert KB/s for this action */
        if( strstr(line, "KB/s: ")
        &&  ! strstr(line, "inf") && ! strstr(line, "nan") )
        {
            snprintf(tmp, 100, "%s", &line[7]);
            if( tmp[strlen(tmp)-1]=='\n')
                tmp[strlen(tmp)-1]='\0';
            utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->transfer_store), &iter, 3, utf8, -1);
            if( utf8 != NULL )
                g_free(utf8);
        }

        /* Scroll to the next client line if this is the KB/s line.
           Otherwise we are already on the client line and there was no KB/s line */
        if( strstr(line, "KB/s: ") )
        while(fgets(line, MAX_READ_POPEN, fp) != NULL)
            break;

        if( strstr(line, "client: ") )
        {
            /* Insert Client address */
            sscanf(line, "%*s %s", tmp);
            utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->transfer_store), &iter, 6, utf8, -1);
            if( utf8 != NULL )
                g_free(utf8);
        }

        /* Scroll to the next line if this is the client line */
        if( strstr(line, "client: ") )
        while(fgets(line, MAX_READ_POPEN, fp) != NULL)
            break;

        if( strstr(line, "server: ") )
        {
            /* Insert Server address */
            sscanf(line, "%*s %s", tmp);
            utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->transfer_store), &iter, 7, utf8, -1);
            if( utf8 != NULL )
                g_free(utf8);
        }

        /* Scroll to the next line if this is a "server: " line */
        if( strstr(line, "server: ") )
        while(fgets(line, MAX_READ_POPEN, fp) != NULL)
            break;

        /* Scroll to the next line if this is a "protocol: " line */
        if( strstr(line, "protocol: ") )
        while(fgets(line, MAX_READ_POPEN, fp) != NULL)
            break;

        /* Insert directory path */
        if( strstr(line, "location: ") )
        {
            /* Scroll to the beginning of the path */
            for(i = 0; line[i]!='\0' && i < 16384; i++)
                if( line[i]!=' ' )
                    break;
            for(i = i; line[i]!='\0' && i < 16384; i++)
                if( line[i]==' ' )
                    break;
            for(i = i; line[i]!='\0' && i < 16384; i++)
                if( line[i]!=' ' )
                    break;

            snprintf(tmp, 16384, "%s", &line[i]);
            if( tmp[strlen(tmp)-1]=='\n' )
                tmp[strlen(tmp)-1]='\0';

            utf8 = g_locale_to_utf8(tmp, strlen(tmp), NULL, NULL, NULL);
            gtk_list_store_set(GTK_LIST_STORE(widgets->transfer_store), &iter, 4, utf8, -1);
            if( utf8 != NULL )
                g_free(utf8);
        }
    }
    pclose(fp);
    free(line);
    free(tmp);

    /* Total bandwidth usage */
    if( totalxfer >= 0 )
    {
        info = g_strdup_printf("  %s %li", _("Total Kbytes/sec:"), totalxfer);
        utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);

        gtk_label_set_text(GTK_LABEL(widgets->total_bandwidth_label), utf8);

        g_free(info);
        if( utf8 != NULL )
            g_free(utf8);
    }

    /* Total incoming xfers */
    if( inxfer >= 0 )
    {
        info = g_strdup_printf("\t %s %li", _("Incoming Kbytes/sec:"), inxfer);
        utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);

        gtk_label_set_text(GTK_LABEL(widgets->total_incoming_label), utf8);

        g_free(info);
        if( utf8 != NULL )
            g_free(utf8);
    }

    /* Total outgoing xfers */
    if( outxfer >= 0 )
    {
        info = g_strdup_printf("\t %s %li  ", _("Outgoing Kbytes/sec:"), outxfer);
        utf8 = g_locale_to_utf8(info, strlen(info), NULL, NULL, NULL);

        gtk_label_set_text(GTK_LABEL(widgets->total_outgoing_label), utf8);

        g_free(info);
        if( utf8 != NULL )
            g_free(utf8);
    }

    return (TRUE);
}
