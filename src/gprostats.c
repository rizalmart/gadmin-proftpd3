/* GADMIN-PROFTPD - GProStats - Statistics generator.
 * Copyright 2001 - 2013 Magnus Loef <magnus-swe@telia.com>
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
 * This is how it works:
 * ------------------------------
 * Takes all users in proftpd.conf and if one if those users exists 
 * in the xferlog those ul/dl stats gets added to that user.
 * The statistics are sorted by top 10 ul and dl.
 * It can generate html output and welcome messages for all users
 * that are currently listed like this in proftpd.conf:
 * <Anonymous /dir/path>
 *  User UserName
 * ....
 * </Anonymous>
 *
*/



#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
/* For Solaris */
#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "allocate.h"


struct user_stats
{
    char user[8192];
    char dir[8192];
    int ul_flag;
    int dl_flag;
    int update;
    uint64_t ul;
    uint64_t dl;
};

/* Private chars are digits function. */
int chars_are_digits_privfunc(char *buf)
{
    int i = 0, match = 0;
    for(i = 0; buf[i] != '\0'; i++)
    {
        if( buf[i] == '\n' || buf[i] == '\r' )
        {
            i--;
            break;
        }
        if( buf[i] == '0' || buf[i] == '1' || buf[i] == '2'
        ||  buf[i] == '3' || buf[i] == '4' || buf[i] == '5'
        ||  buf[i] == '6' || buf[i] == '7' || buf[i] == '8'
        ||  buf[i] == '9' )
            match++;
    }
    if( match && match == i )
        return 1;
    else
        return 0;
}


void show_usage(char conf[8192], char xferlog[8192])
{
    printf("\nGProStats Usage:\n\n");
    printf("-help   Show this help.\n");
    printf("-c      %s\n", conf);
    printf("-x      %s\n", xferlog);
    printf("-w      Make welcome messages for all ftpusers with this name.\n");
    printf("-html   Make a html ftp statistics page in the specified location.\n");
    printf("\tIE: [/var/www/html/ftp.htm]\n\n");

    printf("The default paths are [/etc/proftpd.conf] and [/var/log/xferlog]\n");
    printf("The default welcome for gadmin-proftpd is [welcome.msg]\n\n");

    printf("If neither of the -w or -html flags are used it will just print the results to the screen.\n");
    printf("Only uploads and downloads above 1023 Bytes are counted.\n\n");
}


int main(int argc, char *argv[])
{
    FILE *fp;
    struct user_stats *userlist;
    long conf_size;
    uint64_t file_size;
    char *old_buffer, *user, *dir, *action, *file;
    char *datestamp, *welcome, *welcome1, *temp, *convert;

    /* Filepaths (limited to max 8000) */
    char conf[8192] = "", xferlog[8192] = "", html[8192] = "", welcome_name[8192] = "";

    /* Path to the users directory (+20 for welcome.msg and '/') (8192/8192+20) */
    char user_welcome[16384 + 20] = "";

    /* The name of the server */
    char server_name[8192] = "";

    /* HTML Part 1 (the lengths are max input from the entries) */
    /* Can contain: static HTML(16384), username(128)*20 (2560), ul/dl KB(50)*20 (1000), 
     * server name (1000), datestamp (100) and html_welcome1 */
    char html_welcome[30000] = "";

    /* HTML Part 2 (the lengths are max input from the entries)
     * Can contain: static HTML(3000), username(128)*10 (1280), dl KB(50)*10 (500) */
    char html_welcome1[5000] = "";

    int row = 0, old_row = 0, max_rows = 0, count = 0;
    int x = 0, z = 0, top_ul = 0, top_dl = 0, timez = 0, timex = 0, once = 0;
    int conf_changed = 0, xferlog_changed = 0;
    int found_virtualhost = 0;

    /* Default settings */
    strcpy(conf, PROFTPD_CONF);
    strcpy(xferlog, XFER_LOG);

    /* Check length of arguments */
    for(x = 0; argv[x] != NULL; x++)
    {
        if( strlen(argv[x]) > 8000 )
        {
            printf("\nGProStats error: supplied argument is too long\n");
            show_usage(conf, xferlog);
            exit(1);
        }
        if( x > 1000 )
        {
            printf("\nGProStats error: Max argument reached at 1000\n");
            show_usage(conf, xferlog);
            exit(1);
        }
        if( strstr(argv[x], "%") )
        {
            printf("\nGProStats error: supplied percent char in argument is invalid.\n");
            show_usage(conf, xferlog);
            exit(1);
        }
        if( strstr(argv[x], "0x") )
        {
            printf("\nGProStats error: supplied 0x chars in argument in invalid.\n");
            show_usage(conf, xferlog);
            exit(1);
        }
    }


    printf("\n");

    /* Check for user-supplied args */
    for(x = 0; x < argc; x++)
    {
        if( strstr(argv[x], "-help") )
        {
            show_usage(conf, xferlog);
            exit(1);
        }
        if( argc > x + 1 )
            if( strstr(argv[x], "-c") && strstr(argv[x + 1], "/") )
            {
                conf_changed = 1;
                printf("Using supplied conf path: %s\n", argv[x + 1]);
                strcpy(conf, argv[x + 1]);
            }
        if( argc > x + 1 )
            if( strstr(argv[x], "-x") && strstr(argv[x + 1], "/") )
            {
                xferlog_changed = 1;
                printf("Using supplied xferlog path: %s\n", argv[x + 1]);
                strcpy(xferlog, argv[x + 1]);
            }
        if( argc > x + 1 )
            if( strstr(argv[x], "-w") && strlen(argv[x + 1]) > 0 )
            {
                printf("Creating welcome messages as: %s\n", argv[x + 1]);
                strcpy(welcome_name, argv[x + 1]);
            }
        if( argc > x + 1 )
            if( strstr(argv[x], "-html") && strstr(argv[x + 1], "/") )
            {
                printf("Making a html statistics page here: %s\n", argv[x + 1]);
                strcpy(html, argv[x + 1]);
            }
    }

    /* Show the paths if new paths wherent supplied */
    if( ! conf_changed )
        printf("Using config  path: %s\n", conf);
    if( ! xferlog_changed )
        printf("Using xferlog path: %s\n", xferlog);
    printf("\n");

    /* Get usernames and homedirectories */
    if((fp = fopen(conf, "r")) == NULL)
    {
        printf("\nproftpd.conf could not be found here: %s\n", conf);
        show_usage(conf, xferlog);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    old_buffer = allocate(conf_size);

    if( conf_size > 1 )
        while(fgets(old_buffer, conf_size, fp) != NULL)
        {
            if( strlen(old_buffer) > 8000 )
                continue;

            if( strstr(old_buffer, "<Anonymous /") )
                max_rows++;

            if( strstr(old_buffer, "<VirtualHost") )
                found_virtualhost = 1;

            /* Get the servername for the default server */
            if( ! found_virtualhost )
            {
                if( strstr(old_buffer, "ServerName") )
                {
                    snprintf(server_name, 8000, "%s", old_buffer + 11);
                    server_name[strlen(server_name) - 1] = '\0';
                }
                if( strstr(old_buffer, "ServerIdent on") )
                {
                    snprintf(server_name, 8000, "%s", old_buffer + 15);
                    server_name[strlen(server_name) - 1] = '\0';
                }
            }
        }

    /* Allocate the elements of struct user_stats */
    userlist = malloc(sizeof(struct user_stats) * max_rows);

    user = allocate(8192);
    dir  = allocate(8192);
    rewind(fp);

    if( conf_size > 1 )
        while(fgets(old_buffer, conf_size, fp) != NULL)
        {
            if( strlen(old_buffer) > 8000 )
                continue;

            if( strstr(old_buffer, "<Anonymous /") )
            {
                /* Homedirectories */
                snprintf(dir, 8000, "%s", old_buffer + 11);
                for(x = 0; dir[x]; x++)
                {
                    if( dir[x] == '>' )
                    {
                        dir[x] = '\0';
                        break;
                    }
                }
                snprintf(userlist[row].dir, 8000, "%s", dir);

                while(fgets(old_buffer, conf_size, fp) != NULL)
                {
                    if( strlen(old_buffer) > 8000 )
                        continue;

                    if( strstr(old_buffer, "</Anonymous>") )
                        break;

                    /* Username */
                    if( strstr(old_buffer, "User ")
                    &&  ! strstr(old_buffer, "DirFakeUser")
                    &&  ! strstr(old_buffer, "AllowUser")
                    &&  ! strstr(old_buffer, "DenyUser") )
                    {
                        sscanf(old_buffer, "%*s %s", user);

                        /* Trim off browser added '@' char if any */
                        if( user[strlen(user) - 1] == '@' )
                            user[strlen(user) - 1] = '\0';

                        snprintf(userlist[row].user, 8000, "%s", user);

                        /* Init all to be updated */
                        userlist[row].update = 1;

                        /* Locked .. dont update the welcome message  */
                        while(fgets(old_buffer, conf_size, fp) != NULL)
                        {
                            if( strlen(old_buffer) > 8000 )
                                continue;

                            if( strstr(old_buffer, "#gplockstats") )
                            {
                                userlist[row].update = 0;
                                row--;
                            }
                            if( strstr(old_buffer, "</Anonymous>") )
                                break;
                        }
                        row++;
                        break;
                    }
                }
            }
        }
    fclose(fp);
    free(old_buffer);

    /* Add users ul/dl statistics to the array */
    action = allocate(8192);
    file   = allocate(8192);

    datestamp = allocate(8192);
    temp      = allocate(8192);

    if((fp = fopen(xferlog, "r")) == NULL)
    {
        printf("\nThe xferlog could not be found here: %s\n", xferlog);
        show_usage(conf, xferlog);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    conf_size = ftell(fp);
    rewind(fp);

    old_buffer = allocate(conf_size);

    if( conf_size > 1 )
        while(fgets(old_buffer, conf_size, fp) != NULL)
        {
            if( strlen(old_buffer) > 8000 )
                continue;

            /* DateStamp, 1-5 in the xferlog on the first line that has content */
            if( ! once && strlen(old_buffer) > 5 )
            {
                for(x = 0; old_buffer[x] != '\n'; x++)
                {
                    if( x > 100 )
                    {
                        printf("Date is not in the correct place in the xferlog: %s\n", &old_buffer[0]);
                        break;
                    }

                    if( old_buffer[x] == ' ' && old_buffer[x + 1] != ' ' )
                        z++;

                    if( z == 5 )
                    {
                        /* Security bug fixed thanks to Tavis Ormandy */
                        snprintf(datestamp, x + 1, "%s", old_buffer);
                        once = 1;
                        break;
                    }
                }
            }

            strcpy(user, "");
            file_size = 0;

            /* User */
            sscanf(old_buffer, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %s", user);

            /* Trim off browser added '@' char if any */
            if( user[strlen(user) - 1] == '@' )
                user[strlen(user) - 1] = '\0';

            /* Size */
            strcpy(temp, "");
            sscanf(old_buffer, "%*s %*s %*s %*s %*s %*s %*s %s", temp);
            if( chars_are_digits_privfunc(temp) )
                sscanf(old_buffer, "%*s %*s %*s %*s %*s %*s %*s %llu", &file_size);

            /* UL or DL */
            sscanf(old_buffer, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %s", action);

            /* File */
            sscanf(old_buffer, "%*s %*s %*s %*s %*s %*s %*s %*s %s", file);

            /* Go thru the array looking for this user and add its ul/dl stats */
            row = 0;
            while(row < max_rows)
            {
                if( strcmp(userlist[row].user, user) == 0 )
                {
                    /* Upload */
                    if( strstr(action, "i") )
                        userlist[row].ul = userlist[row].ul + file_size / 1024;

                    /* Download */
                    if( strstr(action, "o") )
                        userlist[row].dl = userlist[row].dl + file_size / 1024;
                }
                row++;
            }
        }
    fclose(fp);
    free(old_buffer);
    free(temp);
    free(user);
    free(dir);
    free(action);
    free(file);

    welcome  = allocate(16384);
    welcome1 = allocate(16384);

    strcpy(welcome, "Welcome to ");
    strcat(welcome, server_name);
    strcat(welcome, " \%U\n\n");

    strcat(welcome, "You are user \%N out of a maximum of \%M authorized logins.\n");
    strcat(welcome, "Current time is \%T.\n");
    strcat(welcome, "The administrator can be reached here: \%E\n");

    if( strlen(datestamp) > 3 && strlen(datestamp) < 100 )
    {
        strcat(welcome, "\nStatistics since: ");
        strcat(welcome, datestamp);
    }
    strcat(welcome, "\n\nTop 10 Uploaders:\n_________________\n");

    /* Generate html statistics */
    if( strlen(html) > 1 )
    {
        /* The page and the top 10 ul */
        strcpy(html_welcome, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n");
        strcpy(html_welcome, "<html><head>\n");
        strcat(html_welcome, "<meta http-equiv=\"CONTENT-TYPE\" content=\"text/html; charset=iso-8859-15\">\n");
        strcat(html_welcome, "<title>GADMIN-PROFTPD statistics</title>\n");
        strcat(html_welcome, "<meta name=\"GENERATOR\" content=\"OpenOffice.org 641  (Linux)\">\n");
        strcat(html_welcome, "<meta name=\"CREATED\" content=\"20030324;20033300\">\n");
        strcat(html_welcome, "<meta name=\"CHANGED\" content=\"20030325;11325700\">\n");
        strcat(html_welcome, "<meta name=\"KEYWORDS\" content=\"GADMIN-PROFTPD statistics\">\n");
        strcat(html_welcome, "<meta name=\"charset\" content=\"ISO-8859-1\">\n");
        strcat(html_welcome, "<style>\n");
        strcat(html_welcome, "th{ font-size: 24px; }\n");
        strcat(html_welcome, "td{ font-size: 16px; }\n");
        strcat(html_welcome, "td.left{ vertical-align: left; padding-left: 15%; }\n");
        strcat(html_welcome, "td.right{ vertical-align: right; padding-right: 15%; }\n");
        strcat(html_welcome, "</style></head>\n");
        strcat(html_welcome, "<body lang=\"en-US\" text=\"#DDDDFF\" link=\"#4b6499\" vlink=\"#3b4581\" bgcolor=\"#3c278e\">\n");
        strcat(html_welcome, "<table border=5 width=800 align=center>\n");
        strcat(html_welcome, "<tr>\n");
        strcat(html_welcome, "<td width=800 colspan=1><font size=5><center>Statistics for ");
        strcat(html_welcome, server_name);
        if( strlen(datestamp) > 3 && strlen(datestamp) < 100 )
        {
            strcat(html_welcome, " since: ");
            strcat(html_welcome, datestamp);
        }
        strcat(html_welcome, "\n</center></td>\n");
        strcat(html_welcome, "</tr></table>\n");
        strcat(html_welcome, "<br>\n");
        strcat(html_welcome, "<table border=2 width=600 align=center>\n");
        strcat(html_welcome, "<tr>\n");
        strcat(html_welcome, "<th width=600 colspan=2>Top 10 uploaders</th>\n");
        strcat(html_welcome, "</tr>\n");

        /* The top 10 dl section */
        strcat(html_welcome1, "<table border=2 width=600 align=center>\n");
        strcat(html_welcome1, "<tr>\n");
        strcat(html_welcome1, "<th width=600 colspan=2>Top 10 downloaders</th>\n");
        strcat(html_welcome1, "</tr>\n");
    }
    free(datestamp);

    /* Calculate top 10 UL and top 10 DL */
    x = 0;
    z = 0;
    row = 0;
    count = 0;
    old_row = 0;
    convert = allocate(8192);

    while( 1 )
    {
        if( row > max_rows - 1 )
            row = 0;

        if( count >= max_rows * max_rows )
            break;

        old_row = 0;
        timez = 0;
        timex = 0;

        while( 1 )
        {
            /* Greater or equal UL then any user in the list thats not flagged */
            if( userlist[row].ul >= userlist[old_row].ul
            &&  ! userlist[row].ul_flag
            &&  ! userlist[old_row].ul_flag )
            {
                top_ul = row;
                timez++;
            }
            /* Greater or equal DL then any user in the list thats not flagged */
            if( userlist[row].dl >= userlist[old_row].dl
            &&  ! userlist[row].dl_flag
            &&  ! userlist[old_row].dl_flag )
            {
                top_dl = row;
                timex++;
            }

            /* .. Or if the user is flagged */
            if( userlist[old_row].ul_flag )
                timez++;

            if( userlist[old_row].dl_flag )
                timex++;

            /* TOP UPLOAD */
            if( timez > max_rows - 1
            &&  userlist[top_ul].ul != 0
            &&  userlist[top_ul].ul_flag != 1 )
            {
                z++;
                if( z <= 10 )
                {
                    userlist[top_ul].ul_flag = 1;
                    strcat(welcome, userlist[top_ul].user);
                    strcat(welcome, ": ");

                    sprintf(convert, "%llu", userlist[top_ul].ul);
                    strcat(welcome, convert);
                    strcat(welcome, " KB\n");

                    /* Generate html */
                    if( strlen(html) > 1 )
                    {
                        strcat(html_welcome, "<tr><td class=left width=300>");
                        strcat(html_welcome, userlist[top_ul].user);
                        strcat(html_welcome, "</td>\n");
                        strcat(html_welcome, "<td align=right class=right width=300>");
                        sprintf(convert, "%llu", userlist[top_ul].ul);
                        strcat(html_welcome, convert);
                        strcat(html_welcome, " KB\n");
                        strcat(html_welcome, "</td></tr>\n");
                    }
                }
            }

            /* TOP Download */
            if( timex > max_rows - 1
            &&  userlist[top_dl].dl != 0
            &&  userlist[top_dl].dl_flag != 1 )
            {
                x++;
                if( x <= 10 )
                {
                    userlist[top_dl].dl_flag = 1;
                    strcat(welcome1, userlist[top_dl].user);
                    strcat(welcome1, ": ");
                    sprintf(convert, "%llu", userlist[top_dl].dl);
                    strcat(welcome1, convert);
                    strcat(welcome1, " KB\n");

                    /* Generate html */
                    if( strlen(html) > 1 )
                    {
                        strcat(html_welcome1, "<tr><td class=left width=300>");
                        strcat(html_welcome1, userlist[top_dl].user);
                        strcat(html_welcome1, "</td>\n");
                        strcat(html_welcome1, "<td class=right align=right width=300>");
                        sprintf(convert, "%llu", userlist[top_dl].dl);
                        strcat(html_welcome1, convert);
                        strcat(html_welcome1, " KB\n");
                        strcat(html_welcome1, "</td></tr>\n");
                    }
                }
            }
            if( old_row < max_rows - 1 )
                old_row++;
            else
                break;
        }
        count++;
        row++;
    }
    free(convert);
    strcat(welcome, "\nTop 10 Downloaders:\n");
    strcat(welcome, "___________________\n");
    strcat(welcome, welcome1);
    strcat(welcome, "\n");


    /* Get todays date */
    datestamp = allocate(8192);
    if((fp = popen("date", "r")) == NULL)
    {
        printf("Pipe Fork error [get date 2]\n");
        return 0;
    }
    fflush(fp);
    while(fgets(datestamp, 8192, fp) != NULL)
    {
        if( strlen(datestamp) > 3 && strlen(datestamp) < 100 )
        {
            strcat(welcome, "\nGenerated on ");
            strcat(welcome, datestamp);
            strcat(welcome, "\n");
            break;
        }
    }
    pclose(fp);


    printf("\n%s\n", welcome);


    /* Add welcome messages for all ftp users in proftpd.conf that should be updated */
    if( strlen(welcome_name) > 1 )
    {
        row = 0;

        while( 1 )
        {
            if( strlen(userlist[row].dir) > 0 )
            {
                /* Minimize writing to the same file */
                if( row > 0 && row < max_rows - 1 )
                {
                    if( strcmp(userlist[row].dir, userlist[row - 1].dir) == 0 )
                    {
                        row++;
                        continue;
                    }
                }
                /* Dont write a welcome.msg if theres a "gplock" lockfile present */
                if( userlist[row].update )
                {
                    /* Write the welcome.msg */
                    strcpy(user_welcome, userlist[row].dir);
                    if( user_welcome[strlen(user_welcome) - 1] != '/' )
                        strcat(user_welcome, "/");
                    strcat(user_welcome, welcome_name);
                    if((fp = fopen(user_welcome, "w+")) == NULL)
                    {
                        printf("\nSomething is wrong with proftpd.conf, could not write to this dir:\n%s\n",
                                                                                           userlist[row].dir);
                    }
                    else
                    {
                        fputs(welcome, fp);
                        fclose(fp);
                    }
                }
            }
            if( row < max_rows - 1 )
                row++;
            else
                break;
        }
    }

    /* Output generated html */
    if( strlen(html) > 1 )
    {
        strcat(html_welcome, "</table><br>\n");
        strcat(html_welcome, html_welcome1);
        strcat(html_welcome, "</table><br><br>\n");

        if( strlen(datestamp) > 3 && strlen(datestamp) < 100 )
        {
            strcat(html_welcome, "<center>Statistics generated by GAdmin-PRoFTPD on \n");
            strcat(html_welcome, datestamp);
            strcat(html_welcome, "</center>\n");
        }
        strcat(html_welcome, "</body></html>\n");

        if((fp = fopen(html, "w+")) == NULL)
        {
            printf("\nCould not write: %s\n", html);
            show_usage(conf, xferlog);
            return 0;
        }
        else
        {
            printf("Adding html statistics to: %s\n", html);
            fputs(html_welcome, fp);
            fclose(fp);
        }
    }
    free(welcome);
    free(welcome1);
    free(datestamp);

    /* Free the struct */
    free(userlist);

    if( argc < 2 )
    {
        printf("\nThe output shown was never written to any file.\n\n");

        printf("Use \"-w welcome.msg\" to write welcome messages for all users.\n");
        printf("Use \"-html /var/www/html/ftp.htm\" to write statistics to a html page.\n");
        printf("Both can be used at the same time.\n");
        printf("Use --help for help.\n\n");
    }

    return 0;
}
