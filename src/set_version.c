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
#include <string.h>
#include <stdlib.h>
#include "widgets.h"
#include "gettext.h"
#include "functions.h"
#include "allocate.h"
#include "show_info.h"

extern int MAX_READ_POPEN;
extern char global_version[1024];



void set_version(struct w *widgets)
{
    FILE *fp;
    char *version;
    int i = 0, ver_found = 0;
    gchar *command;
    gchar *ver = NULL;
    gchar *utf8 = NULL;
    gchar *info;
    GdkColor color;

    command = g_strdup_printf("%s -v 2>&1", PROFTPD_BINARY);
    if((fp = popen(command, "r")) == NULL)
    {
        info = g_strdup_printf(_("Set version failed\n"));
        show_info(info);
        g_free(info);
        g_free(command);
        return;
    }

    g_free(command);
    version = allocate(MAX_READ_POPEN);

    while(fgets(version, MAX_READ_POPEN, fp) != NULL)
    {
        if( cmplowercase(version, "proftpd") && strlen(version) < 30 )
        {
            /* Only get the version at the end */
            for(i = strlen(version)-1; version[i]!='\0'; i--)
                if( version[i]==' ' && version[i+1]!=' ' )
                    break;

            ver = g_strdup_printf(_(" Information: PRoFTPD-%s"), &version[i + 1]);
            utf8 = g_locale_to_utf8(ver, strlen(ver) - 1, NULL, NULL, NULL);
            gtk_label_set_text(GTK_LABEL(widgets->version_label), utf8);
            g_free(utf8);
            ver_found = 1;
        }
    }
    pclose(fp);

    if( ! ver_found )
    {
        ver = g_strdup_printf(_(" Proftpd is not installed or not in your path."));
        utf8 = g_locale_to_utf8(ver, strlen(ver), NULL, NULL, NULL);
        gtk_label_set_text(GTK_LABEL(widgets->version_label), utf8);
        g_free(utf8);
        g_free(ver);

        /* Set status color */
        gdk_color_parse("red", &color);
        gtk_widget_modify_fg(widgets->version_label, GTK_STATE_NORMAL, &color);
    }

    /* Used to determine what options to use with various versions of proftpd */
    snprintf(global_version, 1000, "%s", version);

    free(version);
}
