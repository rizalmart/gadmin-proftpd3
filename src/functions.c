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
#include <time.h>
#include <ctype.h>
#include "widgets.h"
#include "allocate.h"
#include "functions.h"
#include "show_info.h"
#include "system_defines.h"
#include "commands.h"

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#if ! defined USE_DARWIN
#ifndef _CRYPT_H
#include <crypt.h>
#endif
#endif


/* Settings value. Written on apply server settings. */
extern int use_virtual_users;

extern gchar *GP_PASSWD_BUF;
extern gchar *GP_SHADOW_BUF;
extern gchar *GP_GROUP_BUF;
extern gchar *GP_GSHADOW_BUF;



/* Compare strings in lower case */
int cmplowercase(char *buf, char *cmpstring)
{
    char *p, *newbuf;
    int retval = 0;

    if( buf == NULL )
        return retval;

    /* Ignore single newline chars */
    if( strlen(buf) < 2 )
        return retval;

    newbuf = allocate(strlen(buf) + 1);
    /* Exists if it cant allocate */
    sprintf(newbuf, "%s", buf);

    for(p = newbuf; *p != '\0'; ++p)
        *p = tolower(*p);

    if( strstr(newbuf, cmpstring) )
        retval = 1;

    free(newbuf);

    return retval;
}

int chars_are_digits(char *buf)
{
    int i = 0, match = 0;
    for(i = 0; buf[i] != '\0'; i++)
    {
        if( buf[i] == '\n' || buf[i] == '\r' )
        {
            i--;
            break;
        }
        if( buf[i] == '0' || buf[i] == '1' || buf[i] == '2' || buf[i] == '3' || buf[i] == '4'
        ||  buf[i] == '5' || buf[i] == '6' || buf[i] == '7' || buf[i] == '8' || buf[i] == '9' )
            match++;
    }
    if( match && match == i )
        return 1;
    else
        return 0;
}

int commented(char *line)
{
    int i, ret = 0;

    if( line != NULL && strlen(line) > 0 )
    {
        for(i = 0; line[i] != '\0'; i++)
        {
            if( i > 9000 )
            {
                ret = 1;
                break;
            }
            /* Skip whitespace */
            if( line[i] == ' ' || line[i] == '\t' )
                continue;

            /* If the first char thats not whitespace is '#' or ';'
               then the line is commented out */
            if( line[i] == '#' || line[i] == ';' )
                ret = 1;
            else
                ret = 0;

            break;
        }
    }

    return ret;
}

int user_exists(G_CONST_RETURN gchar * username)
{
    /* Checks if the system user exists */
    FILE *fp;
    long conf_size;
    int x, user_exists = 0;
    char tempname[4096] = "";
    char *check_buffer;

    /* Checks if the user exists in passwd */
    if((fp = fopen(GP_PASSWD_BUF, "r")) == NULL)
    {
        /* Dont show anything */
    }
    else
    {
        fseek(fp, 0, SEEK_END);
        conf_size = ftell(fp);
        rewind(fp);

        check_buffer = allocate(conf_size);

        if(conf_size > 1)
        while(fgets(check_buffer, conf_size, fp) != NULL)
        {
            for(x = 0; check_buffer[x]; x++)
            {
                if( check_buffer[x] == ':' )
                {
                    sprintf(tempname, "%s", check_buffer);
                    tempname[x] = '\0';
                    if( strcmp(username, tempname) == 0 )
                    {
                        user_exists = 1;
                        break;
                    }
                }
                if( check_buffer[x] == '\0'
                ||  check_buffer[x] == '\n' )
                    break;
            }
        }
        free(check_buffer);
        fclose(fp);
    }

    /* Checks if the user exists in shadow */
    if((fp = fopen(GP_SHADOW_BUF, "r")) == NULL)
    {
        /* Dont show anything */
    }
    else
    {
        fseek(fp, 0, SEEK_END);
        conf_size = ftell(fp);
        rewind(fp);

        check_buffer = allocate(conf_size);

        if(conf_size > 1)
        while(fgets(check_buffer, conf_size, fp) != NULL)
        {
            for(x = 0; check_buffer[x]; x++)
            {
                if( check_buffer[x] == ':' )
                {
                    sprintf(tempname, "%s", check_buffer);
                    tempname[x] = '\0';
                    if( strstr(username, tempname)
                    &&  strlen(username) == strlen(tempname) )
                    {
                        user_exists = 1;
                        break;
                    }
                }
                if( check_buffer[x] == '\0'
                ||  check_buffer[x] == '\n' )
                    break;
            }
        }
        free(check_buffer);
        fclose(fp);
    }

    return user_exists;
}


int group_exists(G_CONST_RETURN gchar * groupname)
{
    /* Checks if the system group already exists */
    FILE *fp;
    long conf_size;
    int x, group_exists = 0;
    char tempname[4096] = "";
    char *check_buffer;

    /* Checks if the group exists in group */
    if((fp = fopen(GP_GROUP_BUF, "r")) == NULL)
    {
        /* Dont show anything  */
    }
    else
    {
        fseek(fp, 0, SEEK_END);
        conf_size = ftell(fp);
        rewind(fp);
        check_buffer = allocate(conf_size);

        if( conf_size > 1 )
        while(fgets(check_buffer, conf_size, fp) != NULL)
        {
            for(x = 0; check_buffer[x]; x++)
            {
                if( check_buffer[x] == ':' )
                {
                    sprintf(tempname, "%s", check_buffer);
                    tempname[x] = '\0';
                    if( strcmp(groupname, tempname) == 0 )
                    {
                        group_exists = 1;
                        break;
                    }
                }
                if( check_buffer[x] == '\0'
                ||  check_buffer[x] == '\n' )
                    break;
            }
        }
        free(check_buffer);
        fclose(fp);
    }

    /* Checks if the group exists in gshadow */
    if((fp = fopen(GP_GSHADOW_BUF, "r")) == NULL)
    {
        /* Dont show anything */
    }
    else
    {
        fseek(fp, 0, SEEK_END);
        conf_size = ftell(fp);
        rewind(fp);
        check_buffer = allocate(conf_size);

        if(conf_size > 1)
        while(fgets(check_buffer, conf_size, fp) != NULL)
        {
            for(x = 0; check_buffer[x]; x++)
            {
                if( check_buffer[x] == ':' )
                {
                    sprintf(tempname, "%s", check_buffer);
                    tempname[x] = '\0';

                    if( strstr(groupname, tempname)
                    && strlen(groupname) == strlen(tempname) )
                    {
                        group_exists = 1;
                        break;
                    }
                }
                if( check_buffer[x] == '\0'
                ||  check_buffer[x] == '\n' )
                    break;
            }
        }
        free(check_buffer);
        fclose(fp);
    }

    return group_exists;
}


gchar * get_user_setting(gchar * user, gchar * what)
{
    /* Gets a specific users setting from passwd */
    FILE *fp;
    long conf_size;
    int x = 0, y = 0, found_user = 0, found_val = 0, count = 0;
    char *tempname;
    char *check_buffer;
    gchar *groupnumber;
    gchar *retval = NULL;
    gint val_pos = 0;

    /* This is the group number */
    if( strstr(what, "group") )
        val_pos = 3;

    if( strstr(what, "comment") )
        val_pos = 4;

    if( strstr(what, "homedir") )
        val_pos = 5;

    if( strstr(what, "shell") )
        val_pos = 6;

    tempname = allocate(4096);
    if((fp = fopen(GP_PASSWD_BUF, "r")) == NULL)
    {
        retval = g_strdup_printf(_("None"));
        free(tempname);
        return retval;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);
    check_buffer = allocate(conf_size);

    if( conf_size > 1 )
    while(fgets(check_buffer, conf_size, fp) != NULL)
    {
        for(x = 0; check_buffer[x]; x++)
        {
            if( check_buffer[x] == ':' )
            {
                snprintf(tempname, 4000, "%s", check_buffer);
                tempname[x] = '\0';

                if( strcmp(user, tempname) == 0 )
                {
                    found_user = 1;
                    break;
                }
            }
            if( check_buffer[x] == '\0'
            ||  check_buffer[x] == '\n' )
                break;
        }
        if( found_user )
            break;
    }
    fclose(fp);

    /* If the user is found, get the specified section */
    if( found_user )
    for(x = 0; check_buffer[x]; x++)
    {
        if( check_buffer[x] == '\0'
        ||  check_buffer[x] == '\n' )
            break;

        if( check_buffer[x] == ':' )
        {
            count++;
            /* The val_pos colon is found, beginning of the value to get */
            if( count == val_pos )
            {
                snprintf(tempname, 4000, "%s", &check_buffer[x + 1]);
                break;
            }
        }
    }
    free(check_buffer);

    /* Snip at value end, shell comes last and should be newline terminated */
    if(found_user)
    for(y = 0; tempname[y]; y++)
    {
        if( (tempname[y] == ':' && y > 0)
        ||  (tempname[y] == '\n' && y > 0) )
        {
            tempname[y] = '\0';
            /* Atleast one char in the value and max 350 */
            if( strlen(tempname) < 351
            &&  ! strstr(tempname, ":") )
            {
                /* Convert the groupnumber to groupname */
                if( strstr(what, "group") )
                {
                    groupnumber = g_strdup_printf("%s", tempname);
                    retval = get_group_name(groupnumber);
                    g_free(groupnumber);
                }
                else
                    retval = g_strdup_printf("%s", tempname);

                found_val = 1;
                break;
            }
        }
    }
    free(tempname);

    /* g_free'd after the return */
    if( found_val )
        return retval;

    retval = g_strdup_printf(_("None"));
    return retval;
}


gchar * get_group_name(gchar * groupnr)
{
    /* Gets the system group name from the system group number */
    FILE *fp;
    long conf_size;
    int x = 0, y = 0, found_group_number = 0;
    int count = 0, found_group_name = 0;
    char *line, *temp;
    gchar *retval = NULL;
    gint val_pos = 0;

    /* position of the group number (name is always first) */
    val_pos = 2;

    temp = allocate(4096);
    if((fp = fopen(GP_GROUP_BUF, "r")) == NULL)
    {
        retval = g_strdup_printf(_("Group file not found"));
        free(temp);
        return retval;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    line = allocate(conf_size + 1);

    if( conf_size > 1 )
    while(fgets(line, conf_size, fp) != NULL)
    {
        count = 0;

        for(x = 0; line[x] != '\0'; x++)
        {
            if( line[x] == ':' )
                count++;

            if( count == val_pos )
                break;
        }

        snprintf(temp, 4000, "%s", &line[x + 1]);

        /* Snip an extra ":" at the end */
        for(y = strlen(temp) - 1; temp[y]; y--)
        {
            if( temp[y] == ':' )
            {
                temp[y] = '\0';
                break;
            }
        }

        if( strcmp(groupnr, temp) == 0 )
        {
            found_group_number = 1;
            break;
        }

    }
    fclose(fp);


    count = 0;

    /* Get the name of this group */
    if( found_group_number )
    for(y = strlen(line) - 1; line[y] != '\0'; y--)
    {
        if( line[y] == ':' && x < 4000 )
            count++;

        if( count == 3 )
        {
            found_group_name = 1;
            snprintf(temp, y + 1, "%s", line);
            break;
        }
    }
    free(line);

    /* g_free'd after the return */
    if( found_group_name )
    {
        retval = g_strdup_printf("%s", temp);
        free(temp);
        return retval;
    }

    free(temp);
    retval = g_strdup_printf(_("None"));

    return retval;
}


void password_virtual_user(G_CONST_RETURN gchar * username, G_CONST_RETURN gchar * password)
{
    FILE *fp;
    long file_size = 0;
    long i = 0, count = 0;
    int match = 0;
    char *line, *tmp, *part, *new_conf;
    gchar *user_line;
    const char *encrypted_password;

    encrypted_password = encrypt_password(password);

    /* Locate the user, get everything but the first parts (username:password:) */

    if((fp = fopen(GP_PASSWD_BUF, "r")) == NULL)
    {
        printf("Error reading: %s\n", GP_PASSWD_BUF);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);
    part = allocate(file_size + 1);
    tmp = allocate(file_size + 1);
    new_conf = allocate(file_size + 1024);

    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        if( strlen(line) < 10 )
            continue;

        count = 0;
        match = 0;

        for(i = 0; line[i] != '\0'; i++)
        if( line[i] == ':' )
        {
            count++;

            /* See if this is the correct username */
            if( count == 1 )
            {
                snprintf(tmp, file_size, "%s", line);
                tmp[i] = '\0';

                /* Do they match */
                if( strcmp(username, tmp) == 0 )
                {
                    match = 1;
                }

                break;
            }
        }

        for(i = i; line[i] != '\0'; i++)
        if(line[i] == ':')
        {
            count++;

            if(match && count == 3)
            {
                snprintf(part, file_size, "%s", &line[i + 1]);
                break;
            }
        }

        if( count == 3 && match )
        {
            user_line = g_strdup_printf("%s:%s:%s", username, encrypted_password, part);
            strcat(new_conf, user_line);
        }
        else
            strcat(new_conf, line);
    }
    fclose(fp);
    free(line);
    free(tmp);
    free(part);

    /* Write the new and changed virtual password file */
    if((fp = fopen(GP_PASSWD_BUF, "w+")) == NULL)
    {
        printf("Error reading: %s\n", GP_PASSWD_BUF);
        return;
    }
    fputs(new_conf, fp);
    fclose(fp);
    free(new_conf);
}


void delete_virtual_user(G_CONST_RETURN gchar * username)
{
    FILE *fp;
    long file_size = 0;
    long i = 0, count = 0;
    int match = 0;
    char *line, *tmp, *part, *new_conf;

    /* Locate the user and remove it */
    if((fp = fopen(GP_PASSWD_BUF, "r")) == NULL)
    {
        printf("Error reading: %s\n", GP_PASSWD_BUF);
        return;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    line = allocate(file_size + 1);
    part = allocate(file_size + 1);
    tmp = allocate(file_size + 1);
    new_conf = allocate(file_size + 1024);

    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        if( strlen(line) < 10 )
            continue;

        count = 0;
        match = 0;

        for(i = 0; line[i] != '\0'; i++)
        if( line[i] == ':' )
        {
            count++;

            /* See if this is the correct username */
            if( count == 1 )
            {
                snprintf(tmp, file_size, "%s", line);
                tmp[i] = '\0';

                /* Do they match */
                if( strcmp(username, tmp) == 0 )
                {
                    match = 1;
                }
                break;
            }
        }

        for(i = i; line[i] != '\0'; i++)
        if( line[i] == ':' )
        {
            count++;

            if( match && count == 3 )
            {
                snprintf(part, file_size, "%s", &line[i + 1]);
                break;
            }
        }

        if( count == 3 && match )
        {
            /* Skip this user line */
        }
        else
            strcat(new_conf, line);
    }
    fclose(fp);
    free(line);
    free(tmp);
    free(part);

    /* Write the new and changed virtual password file */
    if((fp = fopen(GP_PASSWD_BUF, "w+")) == NULL)
    {
        printf("Error reading: %s\n", GP_PASSWD_BUF);
        return;
    }
    fputs(new_conf, fp);
    fclose(fp);
    free(new_conf);
}


void password_user(G_CONST_RETURN gchar * username, G_CONST_RETURN gchar * password)
{
    FILE *fp;
    char *user_pass, *encrypted_pass;

    if( ! user_exists(username) )
        return;

    user_pass = allocate(8192);

    /* Crypt returns a pointer to a static buffer do not free it */
    encrypted_pass = encrypt_password(password);

    if( encrypted_pass == NULL )
    {
        /* Using unencrypted passwords */
        printf("Crypt error: using the chpass(wd) program instead\n");

#if defined USE_LINUX
        sprintf(user_pass, "echo %s:%s | chpasswd", username, password);
#endif


#if defined USE_OPENBSD || USE_NETBSD || USE_FREEBSD
        sprintf(user_pass, "echo %s:%s | chpass", username, password);
#endif


#if defined USE_DARWIN
        /* Someone who knows what can be used .. niutil lines ? */
        printf("Darwin functions are defined in osx_functions, this is a bug\n");
        sprintf(user_pass, "echo passwd/username %s:%s", username, password);
#endif

    }
    else
    {
        /* Using encrypted passwords */

#if defined USE_LINUX
        sprintf(user_pass, "usermod -p '%s' %s", encrypted_pass, username);
#endif

#if defined USE_OPENSBD || USE_NETBSD || USE_FREEBSD || USE_AIX || USE_HPUX
        sprintf(user_pass, "pw usermod -p '%s' %s", encrypted_pass, username);
#endif

#if defined USE_DARWIN
        sprintf(user_pass, "echo manually change the password: passwd/username '%s' %s", encrypted_pass, username);
#endif

    }


    /* Now we have the correct password changing string, change it */
    if((fp = popen(user_pass, "w")) == NULL)
    {
        perror("popen");
//  sprintf(info_buffer, _("Error changing password for user: %s\n"), username);
//      info_window=create_info_window();
//      gtk_widget_show(info_window);
    }
    else
        pclose(fp);

    free(user_pass);
}


char * encrypt_password(G_CONST_RETURN gchar * password)
{
    /* Make an encrypted password using the MD5 algoritm */
    int i = 0, where = 0, randlen = 8;  /* The max MD5 random stringsize is 8 */
    char *rnd_string;
    char salt[13] = "";   /* The total salt length is 12 */

    /* 64 chars */
    char *arr[] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K",
        "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V",
        "W", "X", "Y", "Z", "a", "b", "c", "d", "e", "f", "g",
        "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r",
        "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "2",
        "3", "4", "5", "6", "7", "8", "9", ".", "/"
    };

    srand((unsigned)time(NULL));

    rnd_string = allocate(9);

    while(i < randlen)
    {
        where = rand() % 64;
        strcat(rnd_string, arr[where]);
        i++;
    }

#if defined HAVE_CRYPT
    /* For this to work it must have linked with -lcrypt */
    sprintf(salt, "$1$%s$", rnd_string);
#else
    /* We cant use MD5 salting and the salt can only be 2 chars */
    sprintf(salt, "%s", &rnd_string[6]);
#endif

    free(rnd_string);

    return crypt(password, salt);
}


int is_banned(char *user)
{
    /* Is this user banned */
    FILE *fp;
    char *old_buffer, *user_buffer;
    int ret = 0, i = 0;
    long conf_size;

    if((fp = fopen(GP_FTPUSERS, "r")) == NULL)
    {
        /* if there is no ftpusers the user isnt banned */
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);
    user_buffer = allocate(8192);
    old_buffer = allocate(conf_size);

    if( conf_size > 1 )
    while(fgets(old_buffer, conf_size, fp) != NULL)
    {
        if( strstr(old_buffer, user) )
        {
            sscanf(old_buffer, "%s", user_buffer);
            for(i = 0; user_buffer[i] != '\0'; i++)
            {
                if( user_buffer[i] == '\n'
                ||  user_buffer[i] == '\r' )
                    user_buffer[i] = '\0';
            }
            if( strcmp(user_buffer, user) == 0 )
            {
                ret = 1;
                break;
            }
        }
    }
    free(old_buffer);
    free(user_buffer);
    fclose(fp);

    return ret;
}


void make_dir_chmod(gchar * directory, char perm[128])
{
    gchar *command;

    command = g_strdup_printf("mkdir -p '%s' && chmod %s '%s'", directory, perm, directory);
    if( ! run_command(command) )
        printf("Error running command: %s\n", command);

    g_free(command);
}


int fix_newlines_in_conf()
{
    /* Strip all newlines between directives */
    /* * then add newlines before <Anonymous and <VirtualHost */
    FILE *fp;
    char *old_buffer, *new_buffer;
    long conf_size, allocate_extra = 0;

    /* First remove any newlines first on all lines in the config */
    /* And see how many new chars we will be adding */
    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        printf("Error reading %s for adding newlines\n", PROFTPD_CONF);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    old_buffer = allocate(conf_size + 1024);
    if( old_buffer == NULL )
    {
        printf("Cant allocate enough memory for newlines\n");
        fclose(fp);
        return 0;
    }

    new_buffer = allocate(conf_size + 1024);
    if( new_buffer == NULL )
    {
        printf("Cant allocate enough memory for newlines\n");
        fclose(fp);
        free(old_buffer);
        return 0;
    }

    if( conf_size > 1 )
    while(fgets(old_buffer, conf_size, fp) != NULL)
    {
        if( old_buffer[0] == '\n'
        ||  old_buffer[0] == '\r' )
        {
            /* Dont collect these lines */
        }
        else
            strcat(new_buffer, old_buffer);

        if( strstr(old_buffer, "<Anonymous")
        ||  strstr(old_buffer, "<VirtualHost") )
            allocate_extra++;
    }
    fclose(fp);
    free(old_buffer);
    strcat(new_buffer, "\n");

    /* Write the newline stripped conf */
    if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
    {
        printf("Error reading %s for adding newlines\n", PROFTPD_CONF);
        free(new_buffer);
        return 0;
    }
    fputs(new_buffer, fp);
    fclose(fp);
    free(new_buffer);

    /* Add the newlines */
    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        printf("Error reading %s for adding newlines\n", PROFTPD_CONF);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    old_buffer = allocate(conf_size + 1024);
    if( old_buffer == NULL )
    {
        printf("Cant allocate enough memory for newlines\n");
        fclose(fp);
        return 0;
    }

    new_buffer = allocate(conf_size + 1024 + allocate_extra);
    if( new_buffer == NULL )
    {
        printf("Cant allocate enough memory for newlines\n");
        fclose(fp);
        free(old_buffer);
        return 0;
    }

    if( conf_size > 1 )
    while(fgets(old_buffer, conf_size, fp) != NULL)
    {
        if( strstr(old_buffer, "<Anonymous")
        ||  strstr(old_buffer, "<VirtualHost") )
        {
            strcat(new_buffer, "\n");
            strcat(new_buffer, old_buffer);
        }
        else
            strcat(new_buffer, old_buffer);
    }
    fclose(fp);
    free(old_buffer);

    /* Write the newline added conf */
    if((fp = fopen(PROFTPD_CONF, "w+")) == NULL)
    {
        printf("Error reading %s for adding newlines\n", PROFTPD_CONF);
        free(new_buffer);
        return 0;
    }
    fputs(new_buffer, fp);
    fclose(fp);
    free(new_buffer);

    return 1;
}

void randomize_username(struct w *widgets)
{
    int len;
    char *rnd_string;
    gchar *info;
    gchar *utf8 = NULL;
    G_CONST_RETURN gchar *default_username_length;

    default_username_length = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[7]));

    if( chars_are_digits((char *)default_username_length) )
    {
        if( strlen((char *)default_username_length) < 4 )
            len = atoi((char *)default_username_length);
        else
        {
            info = g_strdup_printf(_("Max randomize username length is 999\n"));
            show_info(info);
            g_free(info);
            return;
        }
    }
    else
    {
        info = g_strdup_printf(_("Randomize username length can only contain digits\n"));
        show_info(info);
        g_free(info);
        return;
    }

    rnd_string = random_string(len);

    /* Insert the randomized username in the username entry */
    utf8 = g_locale_to_utf8(rnd_string, strlen(rnd_string), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[0]), utf8);

    if( utf8 != NULL )
        g_free(utf8);

    free(rnd_string);
}

void randomize_password(struct w *widgets)
{
    char *rnd_string;
    int len;
    gchar *info;
    gchar *utf8 = NULL;
    G_CONST_RETURN gchar *default_password_length;

    default_password_length = gtk_entry_get_text(GTK_ENTRY(widgets->server_set_spinbutton[8]));

    if( chars_are_digits((char *)default_password_length) )
    {
        if( strlen((char *)default_password_length) < 4 )
            len = atoi((char *)default_password_length);
        else
        {
            info = g_strdup_printf(_("Max randomize password length is 999\n"));
            show_info(info);
            g_free(info);
            return;
        }
    }
    else
    {
        info = g_strdup_printf(_("Randomize password length can only contain digits\n"));
        show_info(info);
        g_free(info);
        return;
    }

    rnd_string = random_string(len);

    /* Insert the randomized password in the password entry */
    utf8 = g_locale_to_utf8(rnd_string, strlen(rnd_string), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->user_set_entry[1]), utf8);

    if( utf8 != NULL )
        g_free(utf8);

    free(rnd_string);
}


char * random_string(int len)
{
    /* Returns a randomized string in upper or lower case 
     * depending on what is selected */
    char *rnd_string;
    gchar *info;
    int upper_case = 0;
    int where = 0;
    int i = 0;

    if( len > 256 )
    {
        info = g_strdup_printf(_("Randomize length too long, changed it to 256.\n"));
        show_info(info);
        g_free(info);
        len = 256;
    }

    /* 256 is the max allowed */
    rnd_string = allocate(300);

    /* Uppercase array 106 chars */
    char *u_arr[] = { "A", "B", "C", "D", "E", "F", "G", "H", "I",
        "J", "K", "L", "M", "N", "O", "P", "Q", "R",
        "S", "T", "U", "V", "W", "X", "Y", "Z",
        "a", "b", "c", "d", "e", "f", "g", "h", "i",
        "j", "k", "l", "m", "n", "o", "p", "q", "r",
        "s", "t", "u", "v", "w", "x", "y", "z",
        "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "1", "2", "3", "4", "5", "6", "7", "8", "9"
    };

    /* Lowercase array 53 chars */
    char *l_arr[] = { "a", "b", "c", "d", "e", "f", "g", "h", "i",
        "j", "k", "l", "m", "n", "o", "p", "q", "r",
        "s", "t", "u", "v", "w", "x", "y", "z",
        "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "1", "2", "3", "4", "5", "6", "7", "8", "9"
    };


    /* Give "deep thought" some time to come up with the number "42" :) */
    usleep(300000);

    /* Make the first random nondigit */
    srand((unsigned)time(NULL));
    while( 1 )
    {
        if( upper_case )
        {
            where = rand() % 106;

            if( ! chars_are_digits(u_arr[where]) )
            {
                strcpy(rnd_string, u_arr[where]);
                break;
            }
        }

        if( ! upper_case )
        {
            where = rand() % 53;

            if( ! chars_are_digits(l_arr[where]) )
            {
                strcpy(rnd_string, l_arr[where]);
                break;
            }
        }
    }

    where = 0;
    i = 1;

    usleep(300000);
    srand((unsigned)time(NULL));

    while(i < len)
    {
        if( upper_case )
        {
            where = rand() % 106;
            strcat(rnd_string, u_arr[where]);
        }
        if( ! upper_case )
        {
            where = rand() % 53;
            strcat(rnd_string, l_arr[where]);
        }
        i++;
    }

    return rnd_string;
}


void exec_ssl(char *command, char descr[128])
{
    FILE *fp;
    if((fp = popen(command, "r")) == NULL)
    {
        perror("popen");
        printf("Error description: %s\n", descr);
        printf("Error command: %s\n", command);
        return;
    }
    pclose(fp);
}
