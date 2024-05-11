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
#include <sys/wait.h>
#include <unistd.h>
#include "gettext.h"
#include "widgets.h"
#include "allocate.h"
#include "show_info.h"
#include "functions.h"


extern int MAX_READ_POPEN;



void run_command_show_err(gchar * command)
{
    FILE *fp;
    char *line, *info;

    if((fp = popen(command, "r")) == NULL)
    {
        perror("popen");
        return;
    }
    else
    {
        line = allocate(MAX_READ_POPEN + 2);
        info = allocate(MAX_READ_POPEN + 2);
        while(fgets(line, MAX_READ_POPEN, fp) != NULL)
            strcat(info, line);

        pclose(fp);
        show_info(info);
        free(info);
        free(line);
    }
}


int run_command(gchar * command)
{
    FILE *fp;
    int status = 0, exit_status = 0;

    if((fp = popen(command, "w")) == NULL)
    {
        perror("popen");
        return 0;
    }
    status = pclose(fp);

    exit_status = WEXITSTATUS(status);

    if( exit_status == 1 )
        exit_status = 0;
    else
        exit_status = 1;

    return exit_status;
}

void init_start(struct w *widgets)
{
    gchar *cmd;
    cmd = g_strdup_printf("%s", SYSINIT_START_CMD);

    if( strlen(cmd) > 4 )
    {
        if( ! run_command(cmd) )
        {
            run_command_show_err(cmd);
        }
    }
    if( cmd != NULL )
        g_free(cmd);
}

void init_stop(struct w *widgets)
{
    gchar *cmd;
    cmd = g_strdup_printf("%s", SYSINIT_STOP_CMD);

    if( strlen(cmd) > 4 )
    {
        if( ! run_command(cmd) )
        {
            run_command_show_err(cmd);
        }
    }
    if( cmd != NULL )
        g_free(cmd);
}

int file_exists(char *infile)
{
    FILE *fp;
    if((fp = fopen(infile, "r")) == NULL)
        return 0;

    fclose(fp);
    return 1;
}

/* Is set to 1 if ther is a valid
   uncommented modulepath */
int have_modulepath = 0;

/* Get all ModulePath and LoadModule statements from an included file. */
char * get_included_dynamic_module_statements(char *path)
{
    FILE *fp;
    char *line;
    long file_size = 0;
    char *included_options = NULL;

    if((fp = fopen(path, "r")) == NULL)
    {
        printf("Error reading: [%s]\n", path);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);
    included_options = allocate(file_size + 1);

    if(file_size > 1)
    while(fgets(line, file_size, fp) != NULL)
    {
        /* LoadModule ModuleName is found. */
        if( cmplowercase(line, "modulepath ")
        ||  cmplowercase(line, "loadmodule ") )
        {
            /* This should be a valid ModulePath */
            if( cmplowercase(line, "modulepath ") && ! commented(line) )
                have_modulepath = 1;

            strcat(included_options, line);
        }
    }
    fclose(fp);
    free(line);

    return included_options;
}


/* Main module function:
   Return any ModulePath and LoadModule statements
   from the current or included configurations or NULL.
   Add ModulePath if missing or commented. */
char * get_dynamic_module_statements()
{
    FILE *fp;
    char *line;
    long file_size = 0;
    int i = 0;
    char *includes = NULL, *include_path;
    char *sub_includes = NULL;

    /* Set global have_modulepath to 0 initially */
    have_modulepath = 0;

    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        /* Report no error. */
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);
    include_path = allocate(file_size + 1);
    includes = allocate(file_size + 1);

    if(file_size > 1)
    while(fgets(line, file_size, fp) != NULL)
    {
        if( strlen(line) < 5 )
            continue;

        /* A ModulePath or LoadModule statement is found. */
        if( cmplowercase(line, "modulepath ")
        ||  cmplowercase(line, "loadmodule ") )
        {

            printf("ModulePath or LoadModule statement found.\n");
            strcat(includes, line);
        }

        /* This should be a valid ModulePath */
        if( cmplowercase(line, "modulepath ") && ! commented(line) )
            have_modulepath = 1;

        /* Get any included conf paths and get their ModulePath and Loadmodule statements. */
        if( cmplowercase(line, "include ") && strstr(line, "/") )
        {
            for(i = 0; line[i] != '\0'; i++)
                if(line[i] == '/')
                    break;

            snprintf(include_path, file_size, "%s", &line[i]);
            if(include_path[strlen(include_path) - 1] == '\n')
                include_path[strlen(include_path) - 1] = '\0';

            sub_includes = get_included_dynamic_module_statements(include_path);
            if(sub_includes != NULL)
            {
                includes = realloc(includes, file_size + 1 + strlen(sub_includes) + 1000);
                strcat(includes, sub_includes);
                free(sub_includes);
            }
        }
    }
    fclose(fp);
    free(line);
    free(include_path);

    /* Add ModulePath if missing */
    if( ! have_modulepath )
    {
        printf("No ModulePath found. Adding one if found.\n");

        /* Debian and others */
        if( file_exists("/usr/lib/proftpd") )
        {
            includes = realloc(includes, file_size + 1 + strlen(includes) + 1000);
            strcat(includes, "ModulePath /usr/lib/proftpd\n");
        }
        else /* Fedora and others */
        if( file_exists("/usr/libexec/proftpd") )
        {
            includes = realloc(includes, file_size + 1 + strlen(includes) + 1000);
            strcat(includes, "ModulePath /usr/libexec/proftpd\n");
        }
    }

    return includes;
}


/* Is the requested module in the included path. */
int module_in_include_path(char *path, char module_name[1024])
{
    FILE *fp;
    char *line;
    long file_size = 0;
    int retval = 0;

    if((fp = fopen(path, "r")) == NULL)
    {
        printf("Error reading: [%s]\n", path);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);

    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
//  FIXED      if( commented(line) )
//            continue;

        /* LoadModule ModuleName is found. */
        if( cmplowercase(line, "loadmodule ")
        &&  cmplowercase(line, module_name) )
        {
            retval = 1;
            break;
        }
    }
    fclose(fp);
    free(line);

    return retval;
}


/* If we have line: "LoadModule ModuleName.c" then
   the requested module is used. If an "Include /..../modules.conf"
   statement is encountered then read that included conf and look for
   LoadModule statements matching the requested module. */
int using_included_module(char module_name[1024])
{
    FILE *fp;
    char *line, *include_path;
    long file_size = 0;
    int i = 0, retval = 0;

    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        printf("Error reading: [%s]\n", PROFTPD_CONF);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);
    include_path = allocate(file_size + 1);

    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        if( commented(line) )
            continue;

        /* LoadModule ModuleName is found. */
        if( cmplowercase(line, "loadmodule ")
        &&  cmplowercase(line, module_name) )
        {
            retval = 1;
            break;
        }

        /* Get any included conf paths and see if they load the requested module. */
        if( cmplowercase(line, "include ") && strstr(line, "/") )
        {
            for(i = 0; line[i] != '\0'; i++)
                if(line[i] == '/')
                    break;

            snprintf(include_path, file_size, "%s", &line[i]);
            if( include_path[strlen(include_path)-1]=='\n' )
                include_path[strlen(include_path)-1]='\0';

            if( module_in_include_path(include_path, module_name) )
            {
                retval = 1;
                break;
            }
        }
    }
    fclose(fp);
    free(line);
    free(include_path);

    return retval;
}


/* Check if a module has been compiled in
   or is included via a LoadModule statement. */
int using_module(char module_name[1024])
{
    FILE *fp;
    int retval = 0;
    char *line;
    char modname_popen[1024] = "";
    gchar *cmd;

    /* Snip the last ".c". For "proftpd -V" search. */
    snprintf(modname_popen, 1000, "%s", module_name);
    modname_popen[strlen(modname_popen) - 2] = '\0';

    cmd = g_strdup_printf("%s -V", PROFTPD_BINARY);
    if((fp = popen(cmd, "r")) == NULL)
    {
        /* Locale trouble could be one cause
           so add TLS and other options */
        retval = 1;
    }
    else
    {
        fflush(fp);
        line = allocate(MAX_READ_POPEN + 1);
        while(fgets(line, MAX_READ_POPEN, fp) != NULL)
        {
            if( strlen(line) > 4 && strstr(line, modname_popen) )
            {
                retval = 1;
                break;
            }
        }
        free(line);
        pclose(fp);
    }
    g_free(cmd);

    /* A module is included by a LoadModule statement. */
    if( using_included_module(module_name) )
        retval = 1;

    return retval;
}
