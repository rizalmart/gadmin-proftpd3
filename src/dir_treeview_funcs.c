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
#include "allocate.h"
#include "functions.h"
#include "show_info.h"
#include "dir_treeview_funcs.h"

/* Temporary for option deprecations */
extern char global_version[1024];

/* The current directory beeing added and limited. */
gchar *dir;

/* If row_pos=0 then this toplevel ftp directory is set */
extern gchar *homedir;

/* Wether theres a problem with directory additions. */
extern int global_dir_error;

/* The 18 checkbox values */
gchar *dir_val[19];

/* The users profile to be appended */
extern char *user_profile;

extern long num_rows;
extern int row_pos;



/* Get the number of rows in the treeview */
gboolean num_rows_func(GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, struct w *widgets)
{
    num_rows++;
    return FALSE;
}


void append_limit_cmds()
{
    int i = 0, has_limit = 0;

    /* Set AllowOverwrite before the directory limits */
    if( dir_val[6] )
        strcat(user_profile, "AllowOverwrite on\n");
    else
        strcat(user_profile, "AllowOverwrite off\n");

    /* ALLOW CMDs. Iter the 18 checkboxes */
    strcat(user_profile, "<Limit");
    for(i = 1; i < 19; i++)
    {
        /* The Box is checked, add to Limit allow */
        if( i == 1 && dir_val[i] )
        {
            strcat(user_profile, " LIST NLST ");
            has_limit = 1;
        }

        if( i == 2 && dir_val[i] )
        {
            strcat(user_profile, " STOR STOU ");
            has_limit = 1;
        }

        if( i == 3 && dir_val[i] )
        {
            strcat(user_profile, " APPE ");
            has_limit = 1;
        }

        if( i == 4 && dir_val[i] )
        {
            strcat(user_profile, " RETR ");
            has_limit = 1;
        }

        if( i == 5 && dir_val[i] )
        {
            strcat(user_profile, " RNFR RNTO ");
            has_limit = 1;
        }

        /* 6 is AllowOverwrite, its handled before this function is called */

        if( i == 7 && dir_val[i] )
        {
            strcat(user_profile, " DELE ");
            has_limit = 1;
        }

        if( i == 8 && dir_val[i] )
        {
            strcat(user_profile, " MKD XMKD SITE_MKDIR ");
            has_limit = 1;
        }

        if( i == 9 && dir_val[i] )
        {
            strcat(user_profile, " RMD XRMD SITE_RMDIR ");
            has_limit = 1;
        }

        if( i == 10 && dir_val[i] )
        {
            strcat(user_profile, " SITE ");
            has_limit = 1;
        }

        if( i == 11 && dir_val[i] )
        {
            strcat(user_profile, " SITE_CHMOD ");
            has_limit = 1;
        }

        if( i == 12 && dir_val[i] )
        {
            strcat(user_profile, " SITE_CHGRP ");
            has_limit = 1;
        }

        /* Show modification dates "show time" */
        if( i == 13 && dir_val[i] )
        {
            strcat(user_profile, " MTDM ");
            has_limit = 1;
        }

        /* Print working directory "show working directory" */
        if( i == 14 && dir_val[i] )
        {
            strcat(user_profile, " PWD XPWD ");
            has_limit = 1;
        }

        /* Show sizes */
        if( i == 15 && dir_val[i] )
        {
            strcat(user_profile, " SIZE ");
            has_limit = 1;
        }

        /* Stat */
        if( i == 16 && dir_val[i] )
        {
            strcat(user_profile, " STAT ");
            has_limit = 1;
        }

        /* Change working directory */
        if( i == 17 && dir_val[i] )
        {
            strcat(user_profile, " CWD XCWD ");
            has_limit = 1;
        }

        /* CD up */
        if( i == 18 && dir_val[i] )
        {
            strcat(user_profile, " CDUP XCUP ");
            has_limit = 1;
        }
    }

    /* If theres no limit the server wont start or go off line */
    if( ! has_limit )
        strcat(user_profile, " NOTHING ");

    strcat(user_profile, ">\n AllowAll\n</Limit>\n");


    /* DENY CMDs. Iter the 18 checkboxes */
    has_limit = 0;
    strcat(user_profile, "<Limit");
    for(i = 1; i < 19; i++)
    {
        /* The Box is not checked, add to Limit deny */
        if( i == 1 && ! dir_val[i] )
        {
            strcat(user_profile, " LIST NLST ");
            has_limit = 1;
        }

        if( i == 2 && ! dir_val[i] )
        {
            strcat(user_profile, " STOR STOU ");
            has_limit = 1;
        }

        if( i == 3 && ! dir_val[i] )
        {
            strcat(user_profile, " APPE ");
            has_limit = 1;
        }

        if( i == 4 && ! dir_val[i] )
        {
            strcat(user_profile, " RETR ");
            has_limit = 1;
        }

        if( i == 5 && ! dir_val[i] )
        {
            strcat(user_profile, " RNFR RNTO ");
            has_limit = 1;
        }

        /* 6 is AllowOverwrite, its handled before this function is called */

        if( i == 7 && ! dir_val[i] )
        {
            strcat(user_profile, " DELE ");
            has_limit = 1;
        }

        if( i == 8 && !dir_val[i] )
        {
            strcat(user_profile, " MKD XMKD SITE_MKDIR ");
            has_limit = 1;
        }

        if( i == 9 && ! dir_val[i] )
        {
            strcat(user_profile, " RMD XRMD SITE_RMDIR ");
            has_limit = 1;
        }

        if( i == 10 && ! dir_val[i] )
        {
            strcat(user_profile, " SITE ");
            has_limit = 1;
        }

        if( i == 11 && ! dir_val[i] )
        {
            strcat(user_profile, " SITE_CHMOD ");
            has_limit = 1;
        }

        if( i == 12 && ! dir_val[i] )
        {
            strcat(user_profile, " SITE_CHGRP ");
            has_limit = 1;
        }

        /* Show modification dates "show time" */
        if( i == 13 && ! dir_val[i] )
        {
            strcat(user_profile, " MTDM ");
            has_limit = 1;
        }

        /* Print working directory "show working directory" */
        if( i == 14 && ! dir_val[i] )
        {
            strcat(user_profile, " PWD XPWD ");
            has_limit = 1;
        }

        /* Show sizes */
        if( i == 15 && ! dir_val[i] )
        {
            strcat(user_profile, " SIZE ");
            has_limit = 1;
        }

        /* Stat */
        if( i == 16 && ! dir_val[i] )
        {
            strcat(user_profile, " STAT ");
            has_limit = 1;
        }

        /* Change working directory */
        if( i == 17 && ! dir_val[i] )
        {
            strcat(user_profile, " CWD XCWD ");
            has_limit = 1;
        }

        /* CD up */
        if( i == 18 && ! dir_val[i] )
        {
            strcat(user_profile, " CDUP XCUP ");
            has_limit = 1;
        }
    }

    /* If theres no limit the server wont start or go off line */
    if( ! has_limit )
        strcat(user_profile, " NOTHING ");

    strcat(user_profile, ">\n DenyAll\n</Limit>\n");
}


/* For each in the directory treeview */
gboolean dirs_foreach(GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, struct w *widgets)
{
    gchar *info = NULL;
    int i = 0;

    /* Iter the 18 checkboxes on each row and save the values */
    for(i = 1; i < 19; i++)
        gtk_tree_model_get(model, iter, i, &dir_val[i], -1);

    /* Get the directory on this row */
    gtk_tree_model_get(model, iter, 0, &dir, -1);

    /* Dont add directories with lengths < 5 */
    if( dir == NULL || strlen((char *)dir) < 5 )
    {
        if( strlen((char *)dir) < 1 )
            info = g_strdup_printf(_("Invalid directory path: NONE"));
        else
            info = g_strdup_printf(_("Invalid directory path: %s"), dir);

        show_info(info);
        g_free(info);

        if( dir != NULL )
            g_free(dir);

        global_dir_error = 1;

        /* Return true so it stops iterating */
        return TRUE;
    }

    /* This is the first toplevel home directory */
    if( row_pos == 0 )
    {
        row_pos++;

        /* Set homedir globally */
        homedir = g_strdup_printf("%s", dir);

        /* Need write: 1 2  4 5 6 7 8  10 */
        if( dir_val[1] || dir_val[2] || dir_val[4] || dir_val[5]
        ||  dir_val[6] || dir_val[7] || dir_val[8] || dir_val[10] )
            make_dir_chmod(dir, "0777");
        else
            make_dir_chmod(dir, "0777");

        if( dir != NULL )
            g_free(dir);

        /* Return true so it stops iterating */
        return TRUE;
    }

    row_pos++;

    /* Ignore the first/home dir */
    if( row_pos > 2 )
    {
        /* Add the rest of the users directories */

        /* Need write: 1 2  4 5 6 7 8  10 */
        if( dir_val[1] || dir_val[2] || dir_val[4] || dir_val[5]
        ||  dir_val[6] || dir_val[7] || dir_val[8] || dir_val[10])
            make_dir_chmod(dir, "0777");
        else
            make_dir_chmod(dir, "0777");

        strcat(user_profile, "<Directory ");
        strcat(user_profile, dir);
        strcat(user_profile, ">\n");

        append_limit_cmds();

        strcat(user_profile, "</Directory>\n");
    }

    if( dir != NULL )
        g_free(dir);

    /* Return false to keep foreach func going */
    return FALSE;
}
