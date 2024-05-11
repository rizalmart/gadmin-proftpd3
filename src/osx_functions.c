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



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include "allocate.h"
#include "chars_are_digits.h"

/* This file will be created from nidump passwd / itll then be read
 * and removed (cant use /etc/passwd since its not kept current?) */
#define GP_OSX_PASS_DUMP "/etc/gp_osx_passdump"



int niutil_run_command(char *command)
{
    /* Run a command */
    FILE *fp;
    int i = 0;

    if((fp = popen(command, "r")) == NULL)
    {
        printf("\nError running %s\n", command);
    }
    else
    {
        pclose(fp);
        i = 1;
    }

    return i;
}


int unlink_file(char *entity)
{
    char *command;
    int unlink = 0;

    if( ! file_exists(entity) )
    {
        printf("Unlink: the file [%s] does not exist.\n", entity);
        unlink = 1;
    }
    else
    {
        command = allocate(8192);

        /* Remove the temporary file */
        sprintf(command, "rm -f /etc/%s", entity);
        if( ! niutil_run_command(command) )
        {
            printf("Error running: %s\n", command);
            free(command);
            exit(1);
        }

        free(command);
        unlink = 1;
    }

    return unlink;
}


int niutil_user_exists(G_CONST_RETURN gchar * username)
{
    /* Checks if the system user exists */
    FILE *fp;
    long conf_size;
    int x, user_exists = 0;
    char tempname[4096] = "";
    char *check_buffer, *command;

    command = allocate(8192);

    sprintf(command, "/usr/bin/nidump passwd / > %s", GP_OSX_PASS_DUMP);

    if( ! niutil_run_command(command) )
    {
        free(command);
        return user_exists;
    }

    free(command);

    /* Checks if the user exists in passwd */
    if((fp = fopen(GP_OSX_PASS_DUMP, "r")) == NULL)
    {
        printf("Cannot open the nidump:ed passwd file.\n");
        return user_exists;
    }
    else
    {
        fseek(fp, 0, SEEK_END);
        conf_size = ftell(fp);
        rewind(fp);
        check_buffer = allocate(conf_size);
        while(fgets(check_buffer, conf_size, fp) != NULL)
        {
            for(x = 0; check_buffer[x]; x++)
            {
                if( check_buffer[x] == ':' )
                {
                    sprintf(tempname, check_buffer);
                    tempname[x] = '\0';
                    if( strcmp(username, tempname) == 0 )
                    {
                        user_exists = 1;
                        break;
                    }
                }
                if(check_buffer[x] == '\0' || check_buffer[x] == '\n')
                    break;
            }
            if( user_exists )
                break;
        }
        free(check_buffer);
        fclose(fp);
    }

    unlink_file(GP_OSX_PASS_DUMP);
    return user_exists;
}


int niutil_get_free_uid_above_500_()
{
    FILE *fp;
    int i = 0, begin = 0, end = 0, colon = 0;
    long length, uid = 0, num = 0;
    char *command, *buffy, *tempuid, *gp_err = NULL;

    command = allocate(8192);

    sprintf(command, "/usr/bin/nidump passwd / > %s", GP_OSX_PASS_DUMP);
    if( ! niutil_run_command(command) )
    {
        free(command);
        return uid;
    }

    free(command);

    if( ! file_exists(GP_OSX_PASS_DUMP) )
    {
        printf("\nError: passwd file not found: %s\n", GP_OSX_PASS_DUMP);
        unlink_file(GP_OSX_PASS_DUMP);
        return uid;
    }


    /* Determine what gid is free and above 500 */
    if((fp = fopen(GP_OSX_PASS_DUMP, "r")) == NULL)
    {
        unlink_file(GP_OSX_PASS_DUMP);
        return uid;
    }

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    rewind(fp);

    buffy = allocate(length);

    tempuid = allocate(8192);

    while(fgets(buffy, length, fp) != NULL)
    {
        /* Dont acctept bad input */
        if( strlen(buffy) < 3 || strlen(buffy) > 1024 )
            continue;

        colon = 0;
        num = 0;
        begin = 0;
        end = 0;

        /* Get the beginning of the gid */
        for(i = 0; buffy[i] != '\0'; i++)
        {
            if( buffy[i] == ':' )
                colon++;

            /* We have found the beginning of the uid */
            if( colon == 2 )
            {
                begin = ++i;
                break;
            }
        }

        /* Get the end of the uid */
        for(i = i; buffy[i] != '\0'; i++)
        {
            end++;
            if( buffy[i] == ':' )
            {
                --end;
                /* strncpy(tempgid, &buffy[begin], end); grr */
                sprintf(tempuid, &buffy[begin]);

                /* Terminate it after the last digit */
                tempuid[end] = '\0';

                /* Collect the greatest uid */
                if( chars_are_digits(tempuid) )
                {
                    /* Conversion to a valid long uid_t */
                    num = (uid_t) strtol(tempuid, &gp_err, 10);
                    if( gp_err && *gp_err )
                        printf("Error Converting a valid long uid\n");
                    else
                    if( num > uid )
                        uid = num;

                    if( LONG_MIN == num )
                        printf("LONG_MIN Conversion for uid\n");

                    if( LONG_MAX == num )
                        printf("LONG_MAX Conversion for uid\n");

                    if( errno == ERANGE )
                        printf("Conversion out of range for uid\n");
                }
                /* No need to report a non digit or a negative value */
                break;
            }
        }
    }

    free(buffy);
    free(tempuid);

    unlink_file(GP_OSX_PASS_DUMP);

    /* Add 1 to uid */
    uid++;

    if( uid <= 499 )
        return 500;
    else
        return uid;
}


int niutil_get_free_gid_above_500_()
{
    FILE *fp;
    int i = 0, begin = 0, end = 0, colon = 0;
    long length, gid = 0, num = 0;
    char *command, *buffy, *tempgid, *gp_err = NULL;

    command = allocate(8192);

    sprintf(command, "/usr/bin/nidump passwd / > %s", GP_OSX_PASS_DUMP);
    if( ! niutil_run_command(command) )
    {
        free(command);
        return gid;
    }

    free(command);


    if( ! file_exists(GP_OSX_PASS_DUMP) )
    {
        printf("\nError: passwd file not found: %s\n", GP_OSX_PASS_DUMP);
        return gid;
    }


    /* Determine what gid is free and above 500 */
    if((fp = fopen(GP_OSX_PASS_DUMP, "r")) == NULL)
    {
        return gid;
    }

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    rewind(fp);

    buffy = allocate(length);

    tempgid = allocate(8192);

    while(fgets(buffy, length, fp) != NULL)
    {
        /* Dont acctept bad input */
        if( strlen(buffy) < 3 || strlen(buffy) > 1024 )
            continue;

        colon = 0;
        num = 0;
        begin = 0;
        end = 0;

        /* Get the beginning of the gid */
        for(i = 0; buffy[i] != '\0'; i++)
        {
            if( buffy[i] == ':' )
                colon++;

            /* We have found the beginning of the gid */
            if( colon == 3 )
            {
                begin = ++i;
                break;
            }
        }

        /* Get the end of the gid */
        for(i = i; buffy[i] != '\0'; i++)
        {
            end++;
            if( buffy[i] == ':' )
            {
                --end;
                sprintf(tempgid, &buffy[begin]);
                /* Terminate it after the last digit */
                tempgid[end] = '\0';

                /* Collect the greatest gid */
                if( chars_are_digits(tempgid) )
                {
                    /* Conversion to a valid long gid_t */
                    num = (gid_t) strtol(tempgid, &gp_err, 10);
                    if( gp_err && *gp_err )
                        printf("Error Converting a valid long gid \n");
                    else
                    if( num > gid )
                        gid = num;

                    if( LONG_MIN == num )
                        printf("LONG_MIN Conversion error for gid\n");

                    if( LONG_MAX == num )
                        printf("LONG_MAX Conversion error for gid\n");

                    if( errno == ERANGE )
                        printf("Conversion out of range for gid\n");
                }
                /* No need to report a non digit or a negative value */
                break;
            }
        }
    }

    free(buffy);
    free(tempgid);

    unlink_file(GP_OSX_PASS_DUMP);

    /* Add 1 to gid */
    gid++;

    if( gid <= 499 )
        return 500;
    else
        return gid;
}


int niutil_useradd(G_CONST_RETURN gchar * username, G_CONST_RETURN gchar * shell)
{
    char *command;
    long uid = 0, gid = 0, gp_id = 0;

    command = allocate(8192);

    /* Create the new users database record */
    sprintf(command, "/usr/bin/niutil -create / /users/%s", username);
    if( ! niutil_run_command(command) )
    {
        printf("Error running: %s\n", command);
        free(command);
        exit(1);
    }

    /* Add a comment for this user */
    sprintf(command, "/usr/bin/niutil -createprop / /users/%s realname %s", username, "ftp-user");
    if( ! niutil_run_command(command) )
    {
        printf("Error running: %s\n", command);
        free(command);
        exit(1);
    }

    if( ! (gid = niutil_get_free_gid_above_500_()) )
    {
        printf("\nError: get_free_gid\n");
        free(command);
        exit(1);
    }

    if( ! (uid = niutil_get_free_uid_above_500_()) )
    {
        printf("\nError: get_free_uid\n");
        free(command);
        exit(1);
    }

    /* We want uid's and gid's to be the same */
    if( uid > gid )
        gp_id = uid;
    else
        gp_id = gid;

    /* Set this users gid */
    sprintf(command, "/usr/bin/niutil -createprop / /users/%s gid %li", username, gp_id);
    if( ! niutil_run_command(command) )
    {
        printf("Error running: %s\n", command);
        free(command);
        exit(1);
    }

    /* Set this users uid */
    sprintf(command, "/usr/bin/niutil -createprop / /users/%s uid %li", username, gp_id);
    if( ! niutil_run_command(command) )
    {
        printf("Error running: %s\n", command);
        free(command);
        exit(1);
    }

    /* Set this users homedirectory to /dev/null */
    sprintf(command, "/usr/bin/niutil -createprop / /users/%s home %s", username, "/dev/null");
    if( ! niutil_run_command(command) )
    {
        printf("Error running: %s\n", command);
        free(command);
        exit(1);
    }

    /* Set the name of the user */
    sprintf(command, "/usr/bin/niutil -createprop / /users/%s name %s", username, username);
    if( ! niutil_run_command(command) )
    {
        printf("Error running: %s\n", command);
        free(command);
        exit(1);
    }

    /* Set the users shell */
    sprintf(command, "/usr/bin/niutil -createprop / /users/%s shell %s", username, shell);
    if( ! niutil_run_command(command) )
    {
        printf("Error running: %s\n", command);
        free(command);
        exit(1);
    }

    /*  /usr/bin/niutil -createprop . /users/username change changetime */
    /*  /usr/bin/niutil -createprop . /users/username expire expiretime */

    free(command);

    /* Return true if the user exists */
    return niutil_user_exists(username);
}


int niutil_userdel(G_CONST_RETURN gchar * username)
{
    int i = 0;
    char *command;

    /* Verify that the user really exists */
    if( ! niutil_user_exists(username) )
        return i;

    command = allocate(8192);

    sprintf(command, "/usr/bin/niutil -destroy / /users/%s", username);
    if( ! niutil_run_command(command) )
    {
        printf("Error running: %s\n", command);
    }
    else
        i = 1;

    free(command);

    return i;
}


int niutil_password_user(G_CONST_RETURN gchar * username, G_CONST_RETURN gchar * password)
{
    /* Password a user */
    int i = 0;
    char *command, *encrypted_pass;

    /*  Verify that the user really exists */
    if( ! niutil_user_exists(username) )
        return i;

    /* Crypt return value should not be freed */
    encrypted_pass = encrypt_password(password);

    if( encrypted_pass == NULL )
    {
        printf("\nCrypt failed.\n");
        exit(1);
    }

    command = allocate(8192);

    /* 'Encrypted Password' otherwise the shell might swallow it */
    sprintf(command, "/usr/bin/niutil -createprop / /users/%s passwd '%s'", username, encrypted_pass);
    if( ! niutil_run_command(command) )
    {
        printf("Error running: %s\n", command);
        free(command);
        exit(1);
    }
    else
        i = 1;

    free(command);

    return i;
}
