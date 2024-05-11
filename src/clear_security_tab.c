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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "gettext.h"
#include "widgets.h"
#include "allocate.h"
#include "clear_security_tab.h"
#include "populate_security_tab.h"


void clear_security_tab(struct w *widgets)
{
    /* Clears the proftpd entries in the security log */
    FILE *fp;
    long file_size;
    char *old_buffy, *new_buffy;

    /* Default to the configured security log */
    gchar *secure_log;
    secure_log = g_strdup_printf("%s", SECURE_LOG);

    /* Use the server settings security log if its set */
    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        /* Do nothing */
    }
    else
    {
        fseek(fp, 0, SEEK_END);
        file_size = ftell(fp);
        rewind(fp);
        old_buffy = allocate(file_size + 1024);

        if( file_size > 1 )
        while(fgets(old_buffy, file_size, fp) != NULL)
        {
            if( ! strstr(old_buffy, "SystemLog ") || strstr(old_buffy, "#") )
                continue;

            if( strlen(old_buffy) < 12 )
                continue;

            /* We have a valid security logfile (doesnt have to exist yet) */
            g_free(secure_log);
            secure_log = g_strdup_printf("%s", &old_buffy[10]);
            if( secure_log[strlen(secure_log)-1]=='\n' )
                secure_log[strlen(secure_log)-1]='\0';
            break;
        }
        free(old_buffy);
    }

    if((fp = fopen(secure_log, "r")) == NULL)
    {
        return;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    old_buffy = allocate(file_size + 1024);
    new_buffy = allocate(file_size + 1024);

    /* Get all lines that doesnt have the string proftpd in them */
    if( file_size > 1 )
    while(fgets(old_buffy, file_size, fp) != NULL)
    {
        if(strstr(old_buffy, "proftpd"))
            continue;

        strcat(new_buffy, old_buffy);
    }
    free(old_buffy);
    fclose(fp);

    if((fp = fopen(secure_log, "w+")) == NULL)
    {
        free(new_buffy);
        return;
    }
    fputs(new_buffy, fp);
    fclose(fp);

    free(new_buffy);
    populate_security(widgets);
}
