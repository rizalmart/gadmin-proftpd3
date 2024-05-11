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

#define _GNU_SOURCE
// NEW
#define _XOPEN_SOURCE

#include "gtk/gtk.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "gettext.h"
#include "widgets.h"
#include "allocate.h"
#include "show_info.h"
#include "functions.h"
#include "make_settings_checkbuttons.h"
#include "make_settings_entries.h"
#include "make_settings_spinbuttons.h"
#include "make_settings_combos.h"
#include "user_auth_directives.h"
#include "apply_server_settings.h"
#include "mysql_functions.h"
#include "process_window.h"

#define MYSQL_CLIENT_BINARY "mysql"


#include <termios.h>

void setup_terminal()
{
    struct winsize win;

    system("rm -f /tmp/AAAAAAAAAAA.txt");

    if( ioctl(STDIN_FILENO, TIOCGWINSZ, &win) )
    {
        system("echo NO_WINSIZE > /tmp/AAAAAAAAAAA.txt");
//        return;
    }
    /* Set it to 24 rows and 80 cols */
    win.ws_row = 24;
    win.ws_col = 80;
    
    ioctl(STDIN_FILENO, TIOCSWINSZ, &win);

    ioctl(STDOUT_FILENO, TIOCSWINSZ, &win);
    ioctl(STDERR_FILENO, TIOCSWINSZ, &win);
}



int is_empty(gchar *input)
{
    if( input == NULL || strlen(input) < 1 )
        return 1;
    else
        return 0;
}

int mysql_run_process_cmd(char **new_argv, char **connect_args,
                          struct w *widgets, int cmd_num, char **user_args)
{
    /* Create a pseudo terminal and fork the process */
    int cmd_number = cmd_num; /* Cmd type incremented in mysql_handle_output() */
    int i, masterpty, slavepty, childpid, child_status = 0;
    int retval_select =  0; /* -1 = Error. */
    int child_exit    =  0; /*  1 = Child exited. */
    int cmd_ret       =  0; /* Command return value. */
    const char *slave_pty_name;
    pid_t wait_pid;
    const char *askpass = "SSH_ASKPASS";
    const char *display = "DISPLAY";
    struct timeval timeout;
    timeout.tv_sec = 20; /* 20 Seconds */
    timeout.tv_usec = 0;

setup_terminal();

    masterpty = posix_openpt(O_RDWR|O_NOCTTY);
    if( masterpty == -1 )
    {
        perror("Can not open a pseudo-terminal master");
        return 0;
    }
    if( grantpt(masterpty) !=0 )
    {
        perror("Can not change permission of the pseudo terminal");
        return 0;
    }
    if( unlockpt(masterpty) != 0 )
    {
        perror("Can not unlock pseudo terminal");
        return 0;
    }

setup_terminal();

    childpid = fork();
    if( childpid == 0 )
    {
        /* This is the child process. */

        /* Create a new session. */
        if( setsid() == -1 )
            fprintf(stdout, "Error: setsid\n");

        /* Get the name of the slave pty */
        slave_pty_name = (const char *) ptsname(masterpty);

        /* Open the slave pty */
        slavepty = open(slave_pty_name, O_RDWR);
        if( slavepty == -1 )
        {
            fprintf(stderr, "Error: Cannot open slavepty.\n");
            return 0;
        }
        close(masterpty);

        /* Show command to run. The password is not shown. */
        fprintf(stdout, "Running command:\n");
        for(i=0; new_argv[i]!=NULL; i++)
           fprintf(stdout, "%s ", new_argv[i]);
        fprintf(stdout, "\n");

        /* Unset SSH_ASKPASS and DISPLAY to disable ssh-askpass popup. */
        if( unsetenv(askpass) == -1 )
          fprintf(stderr, "Unable to unset SSH_ASKPASS environment variable.\n");
        if( unsetenv(display) == -1 )
          fprintf(stderr, "Unable to unset DISPLAY environment variable.\n");


/* We need to fake that we have
   a terminal with sizes */
setup_terminal();


        /* Run the command */
        execvp(new_argv[0], new_argv);

        fprintf(stderr, "Could not run command.\n");
        return 0;
    }
    else
    if( childpid < 0 )
    {
        fprintf(stderr, "Can not create child process.\n");
        return 0;
    }

    /* We are the parent */
    do
    {
        if( ! child_exit )
        {
            fd_set readfd;
            FD_ZERO(&readfd);
            FD_SET(masterpty, &readfd);

setup_terminal();

//while(gtk_events_pending())
//        gtk_main_iteration();

            retval_select = select(masterpty+1, &readfd,
                                    NULL, NULL, &timeout);

            timeout.tv_sec = 20; /* Reset timeout to 20 Seconds */

            if( retval_select > 0 )
            {
                if( FD_ISSET(masterpty, &readfd) )
                {
                    if( ! mysql_handle_output(masterpty, connect_args,
                                        widgets, &cmd_number, user_args) )
                    {
                        /* Process error.
                           Close master control pty for the process. */
                        fprintf(stderr, "Process error.\n");
                        close(masterpty);
                        break;
                    }
                }
            }
            wait_pid = waitpid(childpid, &child_status, WNOHANG);

//while(gtk_events_pending())
//        gtk_main_iteration();
        }
        else
            wait_pid = waitpid(childpid, &child_status, 0);

    } while( wait_pid == 0 ||
             (! WIFEXITED(child_status) && ! WIFSIGNALED(child_status)) );

    /* Child has exited, return child status */
    cmd_ret = (child_status & 0xff00) >> 8;

    if( cmd_ret == 0 )
      cmd_ret = 1;
    else
      cmd_ret = 0;

    fprintf(stderr, "Process exited\n");

    return cmd_ret;
}

/* Write password with write() */
int write_password(int fd, gchar *pass)
{
    int retval = 0;
    ssize_t bytes_written = 0;

setup_terminal();

    bytes_written = write(fd, pass, strlen(pass));
    if( bytes_written == -1 )
    {
        fprintf(stderr, "Error: No bytes written.\n");
        retval = 0;
    }
    else
        retval = 1;

    return retval;
}

/* Write MySQL commands using ioctl TIOCSTI calls */
int write_string(int fd, char *string)
{
    int i = 0, tty = 0, retval = 0;
    char a;

    for(i=0; i<strlen(string); i++)
    {
        a = string[i];

//////////////////////////  FIX FIX FIX CANT OPEN TTY ////////////////////////
        tty = open("/dev/tty", O_WRONLY);
        if( tty == -1 )
        {
            fprintf(stderr, "Can not open write tty.\n");
            return 0;
        }

        retval = ioctl(tty, TIOCSTI, &a);
        if( retval == -1 )
            retval = 0;
        else
            retval = 1;

        close(tty);
    }

    return retval;
}

/* Return result of the previous cmd.
   Show process output in a window */
char * read_output(int fd, struct w *widgets)
{
FILE *fp;
//char *foo;
int i = 0;
    int buf_size = 128; 
    int bytes_read = 0; 
    int ret = 0;
    char buf; 
    char *newbuf; 
    char *buffer = allocate(buf_size); 

    /* Wait for output to arrive. */
    usleep(500000);

    /* Set nonblocking */
    fcntl(fd, F_SETFL,
          fcntl(fd, F_GETFL) | O_NONBLOCK);

    do
    {
        /* Read one byte */
        ret = read(fd, &buf, 1);
        if( ret < 1 )
        {

for(i=0; i<strlen(buffer); i++)
{
// Must be fixed...
if( buffer[i] == '\r' )
    buffer[i] = ' ';

if( buffer[i] == 'Ç' )
    buffer[i] = 'H';

if( buffer[i] == '^X' )
    buffer[i] = 'H';

if( buffer[i] == '^T' )
    buffer[i] = 'H';

if( buffer[i] == '^' )
    buffer[i] = 'X';

}
            /* Show process output in a window */
            show_process_window(widgets, buffer);

//foo = allocate(2);
//foo = buf;
//            show_process_window(widgets, foo);

fp = fopen("/tmp/outputbuffer.txt", "a+");
fputs(buffer, fp);
fclose(fp);

//free(foo);

//while( gtk_events_pending() )
//       gtk_main_iteration();

            return buffer;
        }
        buffer[bytes_read] = buf;
        bytes_read++;

        /* Allocate extra memory if needed */
        if( bytes_read >= buf_size -1 )
        {
            buf_size += 128; 
            newbuf = realloc(buffer, buf_size); 
            if( newbuf == NULL )
            {
                free(buffer);
                return NULL;
            }
            buffer = newbuf; 
        }
    }
    while( 1 );

    buffer[strlen(buffer)]='\0';

    return buffer;
}


int mysql_handle_output(int fd, char **connect_args,
                        struct w *widgets, int *cmd_num, char **user_args)
{
    /* Handle process output. */
    char *output;
    gchar *dbname, *usertable, *grouptable;
    gchar *use_database_cmd;
    int retval = 1;

    /* Read initial output */
    output = read_output(fd, widgets);

if( output == NULL )
    return 1;

if( strlen(output) < 3 )
{
    free(output);
    return 1;
}

    /* Supply password */
    if( strstr(output, "Enter password:") )
    {
        if( ! write_password(fd, connect_args[3]) )
        {
            free(output);
            return 0;
        }
        if( ! write_password(fd, "\n") )
        {
            free(output);
            return 0;
        }
        /* Write a newline to get a prompt */
        if( ! write_string(fd, "\n") )
        {
            free(output);
            return 0;
        }
    }
//    else
//        fprintf(stdout, "\nOutput: [%s]\n", output);

    if( strstr(output, "mysql>") )
    {
        fprintf(stdout, "Prompt detected.\n");
    }
    else
    {
//        fprintf(stdout, "No prompt detected.\n");
        free(output);
        return 1;
    }


    free(output);


    /* Add database if missing. */
    if( *cmd_num == 0 )
    {
        retval = write_string(fd, "show databases;\n");

        output = read_output(fd, widgets);

        dbname = g_strdup_printf(connect_args[4]);

        use_database_cmd = g_strconcat("use ", dbname, ";\n",
        NULL);

        if( strstr(output, dbname) )
        {
            fprintf(stdout, "Database exists.\n");

            retval = write_string(fd, use_database_cmd);
        }
        else
        {
            fprintf(stdout, "Creating database gadmintools.\n");

            if( ! mysql_create_database(fd, dbname) )
            {
                if( output != NULL )
                    free(output);
                return 0;
            }
            retval = write_string(fd, use_database_cmd);
        }

        g_free(dbname);
        g_free(use_database_cmd);

        *cmd_num = 1;
        free(output);
        return retval;
    }

    /* Add tables if missing. */
    if( *cmd_num == 1 )
    {
        retval = write_string(fd, "show tables;\n");

        output = read_output(fd, widgets);

        dbname = g_strdup_printf(connect_args[4]);

        usertable  = g_strdup_printf(connect_args[5]);
        grouptable = g_strdup_printf(connect_args[6]);

        if( strstr(output, usertable) )
        {
            fprintf(stdout, "User table exists.\n");

            /* Write a newline to get a prompt.
               Because no command has been used. */
            if( ! write_string(fd, "\n") )
            {
                g_free(dbname); g_free(usertable); g_free(grouptable);
                free(output);
                return 0;
            }
        }
        else
        {
            fprintf(stdout, "Creating database tables.\n");

            if( ! mysql_create_tables(fd, dbname, usertable, grouptable) )
            {
                g_free(dbname); g_free(usertable); g_free(grouptable);
                free(output);
                return 0;
            }
        }
        g_free(dbname);
        g_free(usertable);
        g_free(grouptable);

        /* 100 means quit */
        *cmd_num = 100;
        free(output);
        return 1;
    }



    /* Add user */
    if( *cmd_num == 2 )
    {
/*
        fprintf(stdout, "Deleting user: %s\n", user_args[0]);

        if( ! mysql_delete_user(fd, widgets, connect_args, user_args) )
        {

        }
*/
        fprintf(stdout, "Adding user: %s\n", user_args[0]);

        if( ! mysql_add_user(fd, widgets, connect_args, user_args) )
        {

        }

        *cmd_num = 100; /* Quit */
    }

    /* Delete user */
    if( *cmd_num == 3 )
    {
        fprintf(stdout, "Deleting user: %s\n", user_args[0]);

        if( ! mysql_delete_user(fd, widgets, connect_args, user_args) )
        {

        }

        *cmd_num = 100; /* Quit */
    }

    /* Add group */
    if( *cmd_num == 4 )
    {
        fprintf(stdout, "Adding group: %s\n", user_args[0]);

        if( ! mysql_add_group(fd, widgets, connect_args, user_args, NULL) )
        {

        }

        *cmd_num = 100; /* Quit */
    }

    /* Delete group */
    if( *cmd_num == 5 )
    {
        fprintf(stdout, "Deleting group: %s\n", user_args[0]);

        if( ! mysql_delete_group(fd, widgets, connect_args, user_args) )
        {

        }

        *cmd_num = 100; /* Quit */
    }




    /* Quit */
    if( *cmd_num == 100 )
    {
        if( ! mysql_disconnect(fd) )
            return 0;

        output = read_output(fd, widgets);
        free(output);
    }

    return 1;
}


void mysql_free_connect_args(char **connect_args)
{
    int i = 0;

    /* Free connect_args array data */
    for(i=0; i<7; i++)
        free(connect_args[i]);

    /* Free connect_args array */
    free(connect_args);
}

/* Get connection arguments from proftpd.conf
   Host, port, user, pass, dbname, usertable, grouptable.*/
char ** mysql_get_connect_args()
{
    FILE *fp;
    long file_size = 0;
    int i = 0;
    char *line, *data;
    char **connect_args;

    if((fp = fopen(PROFTPD_CONF, "r")) == NULL)
    {
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    line = allocate(file_size + 1);

/*  This are the lines we collect values from:
    SQLConnectInfo DBNAME@HOSTNAME:PORT USERNAME PASSWORD
    SQLUserInfo USERTABLE username password uid gid homedir shell
    SQLGroupInfo GROUPTABLE groupname gid members
*/
    data = allocate(512);

    /* Allocate connect_args array */
    connect_args = malloc((8) * sizeof(char *));
    memset(connect_args, '\0', sizeof(connect_args));
    /* Allocate connect_args array data */
    for(i=0; i<7; i++)
        connect_args[i] = allocate(512);
    connect_args[7]=NULL;

    if( file_size > 1 )
    while(fgets(line, file_size, fp) != NULL)
    {
        if( commented(line) || strlen(line) > 500 )
            continue;

        if( cmplowercase(line, "sqlconnectinfo ") )
        {
           /* Get dbname */
            sscanf(line, "%*s %s", data);
            snprintf(connect_args[4], 500, "%s", data);
            for(i=0; i<strlen(connect_args[4]); i++)
                if( connect_args[4][i]=='@' )
                {
                    connect_args[4][i]='\0';
                    break;
                }
           /* Get hostname */
            sscanf(line, "%*s %s", data);
            snprintf(connect_args[0], 500, "%s", &data[i+1]);
            for(i=0; i<strlen(connect_args[0]); i++)
                if( connect_args[0][i]==':' )
                {
                    connect_args[0][i]='\0';
                    break;
                }
           /* Get port */
            sscanf(line, "%*s %s", data);
            for(i=0; i<strlen(data); i++)
                if( data[i]==':' )
                {
                    snprintf(connect_args[1], 500, "%s", &data[i+1]);
                    break;
                }
            /* Get username */
            sscanf(line, "%*s %*s %s", data);
            snprintf(connect_args[2], 500, "%s", data);
            /* Get password */
            sscanf(line, "%*s %*s %*s %s", data);
            snprintf(connect_args[3], 500, "%s", data);
        }

        if( cmplowercase(line, "sqluserinfo ") )
        {
            /* Get usertable */
            sscanf(line, "%*s %s", data);
            snprintf(connect_args[5], 500, "%s", data);
        }

        if( cmplowercase(line, "sqlgroupinfo ") )
        {
            /* Get grouptable */
            sscanf(line, "%*s %s", data);
            snprintf(connect_args[6], 500, "%s", data);
        }
    }
    fclose(fp);
    free(line);
    free(data);

    return connect_args;
}


/* cmd_num decides what to do:
   0 = Add database and tables.
   2 = Add user. 3 = delete user.
   4 = Add group. 5 = delete group. */

int mysql_connect(char **connect_args,
                    struct w *widgets, int cmd_num, char **user_args)
{
    int retval = 0;
    char **new_argv;
    char hostline[256]="";
    char portline[256]="";
    char userline[256]="";
    snprintf(hostline, sizeof(hostline), "--host=%s", connect_args[0]);
    snprintf(portline, sizeof(portline), "--port=%s", connect_args[1]);
    snprintf(userline, sizeof(userline), "--user=%s", connect_args[2]);

    new_argv = malloc((11) * sizeof(char *));
    memset(new_argv, '\0', sizeof(new_argv));

    new_argv[0]="strace";
    new_argv[1]="-o";
    new_argv[2]="/tmp/straceout.txt";
    new_argv[3]=MYSQL_CLIENT_BINARY;
    new_argv[4]=hostline;
    new_argv[5]=portline;
    new_argv[6]=userline;
    new_argv[7]="--tee=/dev/tty";
    new_argv[8]="--protocol=TCP";
//    new_argv[8]="--skip-pager";
//    new_argv[9]="--tee=/tmp/mysqlout.txt";
    /* -p asks for the password. Its supplied
       on the command line when connected. */
    new_argv[9]="-p";
    new_argv[10]=NULL;

/*
    new_argv[0]=MYSQL_CLIENT_BINARY;
    new_argv[1]=hostline;
    new_argv[2]=portline;
    new_argv[3]=userline;
    new_argv[4]="--tee=/dev/tty";
*/
    /* -p asks for the password. Its supplied
       on the command line when connected. */
/*
    new_argv[5]="-p"; 
    new_argv[6]=NULL;
*/

    /* Connect args 3 is the password */
    retval = mysql_run_process_cmd(new_argv, connect_args, widgets, cmd_num, user_args);
    if( ! retval )
    {
        free(new_argv);
        return 0;
    }
    free(new_argv);

    return 1;
}

int mysql_disconnect(int fd)
{
    if( ! write_string(fd, "quit\n") )
       return 0;

    fprintf(stderr, "Disconnected from MySQL server.\n");
    return 1;
}

int mysql_user_exists(int fd, struct w *widgets, char **connect_args, char **user_args)
{
    int retval = 0;
    char *output;
    gchar *database_cmd;

    /* Use database */
    database_cmd = g_strconcat("use ", connect_args[4], ";\n",
    NULL);
    retval = write_string(fd, database_cmd);
    output = read_output(fd, widgets);
    free(output);
    g_free(database_cmd);

    database_cmd = g_strconcat(
    "SELECT username FROM ", connect_args[5], " WHERE username=",
    "'", user_args[0], "';\n",
    NULL);

    retval = write_string(fd, database_cmd);
    output = read_output(fd, widgets);

    if( strstr(output, "Empty set") )
        retval = 0;
    else
        retval = 1;

    free(output);
    g_free(database_cmd);

    return retval;
}

int mysql_group_exists(int fd, struct w *widgets, char **connect_args, char **user_args)
{
    int retval = 0;
    char *output;
    gchar *database_cmd;

    /* Use database */
    database_cmd = g_strconcat("use ", connect_args[4], ";\n",
    NULL);
    retval = write_string(fd, database_cmd);
    output = read_output(fd, widgets);
    free(output);
    g_free(database_cmd);

    database_cmd = g_strconcat("SELECT groupname FROM ",
        connect_args[6], " WHERE groupname=",
        "'", user_args[7], "';\n",
    NULL);

    retval = write_string(fd, database_cmd);
    output = read_output(fd, widgets);

    if( strstr(output, "Empty set") )
        retval = 0;
    else
        retval = 1;

    free(output);
    g_free(database_cmd);

    return retval;
}

/* Add or change a user. Adds group with same gid as uid if missing */
int mysql_add_user(int fd, struct w *widgets, char **connect_args, char **user_args)
{
    int retval = 0;
    char *output;
    gchar *database_cmd, *add_user_cmd;
    int i = 0, z = 0, leave = 0, result = 0;
    char *uid;

    /* Use database */
    database_cmd = g_strconcat("use ", connect_args[4], ";\n",
    NULL);

    retval = write_string(fd, database_cmd);
    output = read_output(fd, widgets);
    free(output);
    g_free(database_cmd);

    /* Check if the user exists, if so change its settings */
    if( mysql_user_exists(fd, widgets, connect_args, user_args) )
    {
        printf("User exists. Updating it...\n");
        /* Change a user */
        add_user_cmd = g_strconcat(
        "UPDATE ", connect_args[5], " SET ",
        "password='", user_args[1], "',",
        "homedir='", user_args[4], "',",
        "shell='", user_args[5], "',",
        "comment='", user_args[6], "' ",
        "WHERE username='", user_args[0], "';\n",
        NULL);

        retval = write_string(fd, add_user_cmd);
        output = read_output(fd, widgets);
        free(output);

        g_free(add_user_cmd);

        return 0;
    }

    /* Add a new user... */

    /* Get highest UID/GID */
    database_cmd = g_strconcat(
    "SELECT uid FROM ", connect_args[5], " ORDER BY uid DESC LIMIT 1;\n",
    NULL);
    retval = write_string(fd, database_cmd);
    output = read_output(fd, widgets);

    /* Line by line read */
    for(i=0; output[i]!='\0'; i++)
    {
        for(z=i; output[z]!='\0'; z++)
        {
            if( output[z]=='\n')
            {
                uid = allocate(z+1);
                snprintf(uid, z-i+1, "%s", &output[i-1]);
                fprintf(stdout, "UID Line: [%s]\n", uid);
                sscanf(uid, "%*s %d", &result);
                if( result >= 1000 )
                {
                    fprintf(stdout, "Result: %d\n", result);
                    free(uid);
                    leave = 1;
                    break;
                }
                free(uid);
                i = z + 1;
                break;
            }
        }
        if( leave )
            break;
    }

    if( result >= 1000 )
        result++;
    else
        result = 1000;

    free(output);
    g_free(database_cmd);

    /* Int result to char result */
    uid = allocate(128);
    snprintf(uid, 100, "%d", result);

    /* Add a new user */
    add_user_cmd = g_strconcat(
    "INSERT INTO ", connect_args[5], " ",
    "(username, password, uid, gid, homedir, shell, comment) ",
    "VALUES (",
    "'", user_args[0], "',",
    "'", user_args[1], "',",
    "'", uid, "',",
    "'", uid, "',",
    "'", user_args[4], "',",
    "'", user_args[5], "',",
    "'", user_args[6], "');\n",
    NULL);

    retval = write_string(fd, add_user_cmd);
    output = read_output(fd, widgets);
    free(output);
    g_free(add_user_cmd);

    /* Does the group exist */
    if( mysql_group_exists(fd, widgets, connect_args, user_args) )
    {
        free(uid);
        return 0;
    }

    /* Create the users group with the same gid as uid */
    if( ! mysql_add_group(fd, widgets, connect_args, user_args, uid) )
    {
        printf("Error adding group.\n");
    }

    free(uid);

    /* Avoid compiler warning */
    if( retval )
    { }

    return 0;
}

int mysql_delete_user(int fd, struct w *widgets, char **connect_args, char **user_args)
{
    int retval = 0;
    char *output;
    gchar *database_cmd, *del_user_cmd;

    /* Use database */
    database_cmd = g_strconcat("use ", connect_args[4], ";\n",
    NULL);

    retval = write_string(fd, database_cmd);
    output = read_output(fd, widgets);
    free(output);
    g_free(database_cmd);

    /* Add or change a user */
    del_user_cmd = g_strconcat(
    "DELETE FROM ", connect_args[5], " WHERE username=",
    "'", user_args[0], "';\n",
    NULL);

    retval = write_string(fd, del_user_cmd);
    output = read_output(fd, widgets);
    free(output);

    g_free(del_user_cmd);

    /* Avoid compiler warning */
    if( retval )
    { }

    return 0;
}

int mysql_add_group(int fd, struct w *widgets,
                    char **connect_args, char **user_args, char *gid)
{
    int retval = 0;
    char *output;
    gchar *add_group_cmd;

    /* Add a new user */
    add_group_cmd = g_strconcat(
    "INSERT INTO ", connect_args[6], " ",
    "(groupname, gid, members) ",
    "VALUES (",
    "'", user_args[7], "',",
    "'", gid, "',",
    "'NULL'",");\n",
    NULL);

    retval = write_string(fd, add_group_cmd);
    output = read_output(fd, widgets);
    free(output);

    g_free(add_group_cmd);

    /* Avoid compiler warning */
    if( retval )
    { }

    return 0;
}

/* Can be used later. After deleting a user
   a popup can ask if the group should also be deleted. */
int mysql_delete_group(int fd, struct w *widgets, char **connect_args, char **user_args)
{


    return 0;
}

int mysql_create_database(int fd, gchar *dbname)
{
    gchar *data;
    data = g_strdup_printf("CREATE DATABASE %s;\n", dbname);

    if( ! write_string(fd, data) )
    {
        fprintf(stderr, "Error: Could not create database.\n");
        g_free(data);
        return 0;
    }

    fprintf(stdout, "Wrote new database.\n");
    g_free(data);

    return 1;
}

int mysql_create_tables(int fd, gchar *dbname, gchar *usertable, gchar *grouptable)
{
    gchar *data;

    /* Create user table */
    data = g_strconcat(
        "CREATE TABLE ", usertable, " (",
        "username VARCHAR(255) NOT NULL UNIQUE,",
        "password VARCHAR(128) NOT NULL,",
        "uid INTEGER UNIQUE,",
        "gid INTEGER,",
        "homedir VARCHAR(255),",
        "shell VARCHAR(255),",
        "comment VARCHAR(128) ",
        ");\n",
    NULL);

    if( ! write_string(fd, data) )
    {
        fprintf(stderr, "Error: Could not create user table.\n");
        g_free(data);
        return 0;
    }
    g_free(data);

    /* Create group table */
    data = g_strconcat(
        "CREATE TABLE ", grouptable," (",
        "groupname VARCHAR(255) NOT NULL,",
        "gid INTEGER NOT NULL,",
        "members VARCHAR(1024) ",
        ");\n",
    NULL);

    if( ! write_string(fd, data) )
    {
        fprintf(stderr, "Error: Could not create group table.\n");
        g_free(data);
        return 0;
    }
    g_free(data);

    /* Create quotalimits table */
    data = g_strconcat(
        "CREATE TABLE quotalimits (",
        "name VARCHAR(255),",
        "quota_type ENUM(\"user\", \"group\", \"class\", \"all\") NOT NULL,",
        "per_session ENUM(\"false\", \"true\") NOT NULL,",
        "limit_type ENUM(\"soft\", \"hard\") NOT NULL,",
        "bytes_in_avail FLOAT NOT NULL,",
        "bytes_out_avail FLOAT NOT NULL,",
        "bytes_xfer_avail FLOAT NOT NULL,",
        "files_in_avail INT UNSIGNED NOT NULL,",
        "files_out_avail INT UNSIGNED NOT NULL,",
        "files_xfer_avail INT UNSIGNED NOT NULL",
        ");\n",
    NULL);

    if( ! write_string(fd, data) )
    {
        fprintf(stderr, "Error: Could not create quotalimits table.\n");
        g_free(data);
        return 0;
    }
    g_free(data);


    /* Create quotatallies table */
    data = g_strconcat(
        "CREATE TABLE quotatallies (",
        "name VARCHAR(255) NOT NULL,",
        "quota_type ENUM(\"user\", \"group\", \"class\", \"all\") NOT NULL,",
        "bytes_in_used FLOAT NOT NULL,",
        "bytes_out_used FLOAT NOT NULL,",
        "bytes_xfer_used FLOAT NOT NULL,",
        "files_in_used INT UNSIGNED NOT NULL,",
        "files_out_used INT UNSIGNED NOT NULL,",
        "files_xfer_used INT UNSIGNED NOT NULL",
        ");\n",
    NULL);

    if( ! write_string(fd, data) )
    {
        fprintf(stderr, "Error: Could not create quotatallies table.\n");
        g_free(data);
        return 0;
    }
    g_free(data);

    fprintf(stdout, "Wrote new database tables.\n");

    return 1;
}

void mysql_window_apply_clicked(struct w *widgets)
{
    gchar *host, *port, *user, *pass;
    gchar *database, *usertable, *grouptable;
    gchar *info;

    host       = g_strdup_printf(gtk_entry_get_text(GTK_ENTRY(widgets->remote_server_entry)));
    port       = g_strdup_printf(gtk_entry_get_text(GTK_ENTRY(widgets->remote_port_entry)));
    user       = g_strdup_printf(gtk_entry_get_text(GTK_ENTRY(widgets->remote_user_entry)));
    pass       = g_strdup_printf(gtk_entry_get_text(GTK_ENTRY(widgets->remote_password_entry)));
    database   = g_strdup_printf(gtk_entry_get_text(GTK_ENTRY(widgets->remote_database_entry)));
    usertable  = g_strdup_printf(gtk_entry_get_text(GTK_ENTRY(widgets->remote_usertable_entry)));
    grouptable = g_strdup_printf(gtk_entry_get_text(GTK_ENTRY(widgets->remote_grouptable_entry)));

    /* Password can be empty */
    if( is_empty(host) || is_empty(port) || is_empty(user) || is_empty(database)
    ||  is_empty(usertable) || is_empty(grouptable) )
    {
        info = g_strdup_printf(_("All fields must be filled in.\n"));
        show_info(info);
        if( info!=NULL )
            g_free(info);
        if( pass != NULL )
            g_free(pass);

        g_free(host); g_free(port); g_free(user);
        g_free(database); g_free(usertable); g_free(grouptable);

        return;
    }

    /* Remove any MySQL directives from the conf */
    mysql_user_directives(0, widgets);

    /* Add MySQL directives to the conf */
    mysql_user_directives(1, widgets);

    /* Create database and tables */
    char **connect_args;
    connect_args = mysql_get_connect_args();

    int cmd_num = 0;
    if( ! mysql_connect(connect_args, widgets, cmd_num, NULL) )
        printf("Error: Could not connect to mysql server.\n");

    free(connect_args);


    apply_server_settings(widgets);

    if( pass != NULL )
        g_free(pass);
    g_free(host); g_free(port); g_free(user);
    g_free(database); g_free(usertable); g_free(grouptable);

    gtk_widget_destroy(widgets->mysql_window);
}


void mysql_window_cancel_clicked(struct w *widgets)
{
    gtk_widget_destroy(widgets->mysql_window);
}


void show_mysql_window(struct w *widgets)
{
    GtkWidget *frame1;
    GtkWidget *table1;
    GtkWidget *menu_vbox;
    gchar *utf8=NULL;
    gchar *info, *text;
    int a=0, b=1;

    widgets->mysql_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(widgets->mysql_window), GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request(widgets->mysql_window, 500, -1);

    /* Set window information */
    info = g_strdup_printf(_("GAdmin-PRoFTPD %s MySQL settings"), VERSION);

    gtk_window_set_title(GTK_WINDOW(widgets->mysql_window), info);
    g_free(info);

    menu_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(widgets->mysql_window), menu_vbox);

//    tooltips = gtk_tooltips_new();

    /* 1 tables with some settings and 2 columns */
    table1 = gtk_table_new(2, 2, FALSE);

    frame1 = gtk_frame_new(_("Supply MySQL settings:"));

    gtk_box_pack_start(GTK_BOX(menu_vbox), frame1, TRUE, TRUE, 1);
    gtk_container_add(GTK_CONTAINER(frame1), table1);

    widgets->remote_server_entry   = make_entry_with_label(GTK_TABLE(table1), _(" MySQL server: "), 0,1,a,b, 300);
    a++; b++;
    widgets->remote_port_entry     = make_entry_with_label(GTK_TABLE(table1), _(" MySQL port: "), 0,1,a,b, 300);
    a++; b++;
    widgets->remote_user_entry     = make_entry_with_label(GTK_TABLE(table1), _(" MySQL admin user: "), 0,1,a,b, 300);
    a++; b++;
    widgets->remote_password_entry = make_entry_with_label(GTK_TABLE(table1), _(" MySQL admin password: "), 0,1,a,b, 300);
    a++; b++;
    widgets->remote_database_entry = make_entry_with_label(GTK_TABLE(table1), _(" Database name: "), 0,1,a,b, 300);
    a++; b++;
    widgets->remote_usertable_entry = make_entry_with_label(GTK_TABLE(table1), _(" User table name: "), 0,1,a,b, 300);
    a++; b++;
    widgets->remote_grouptable_entry = make_entry_with_label(GTK_TABLE(table1), _(" Group table name: "), 0,1,a,b, 300);
    a++; b++;

    /* Use asterisks for the password field */
    gtk_entry_set_visibility(GTK_ENTRY(widgets->remote_password_entry), FALSE);

    /* Server */
    text = g_strdup_printf("localhost");
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->remote_server_entry), utf8);
    if( text!=NULL )
      g_free(text);
    if( utf8!=NULL )
      g_free(utf8);

    /* Port */
    text = g_strdup_printf("3306"); /* Standard MySQL port is 3306 */
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->remote_port_entry), utf8);
    if( text!=NULL )
      g_free(text);
    if( utf8!=NULL )
      g_free(utf8);

    /* User */
    text = g_strdup_printf("root");
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->remote_user_entry), utf8);
    if( text!=NULL )
      g_free(text);
    if( utf8!=NULL )
      g_free(utf8);

    /* Database name */
    text = g_strdup_printf("gadmintools");
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->remote_database_entry), utf8);
    if( text!=NULL )
      g_free(text);
    if( utf8!=NULL )
      g_free(utf8);

    /* User table name */
    text = g_strdup_printf("users");
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->remote_usertable_entry), utf8);
    if( text!=NULL )
      g_free(text);
    if( utf8!=NULL )
      g_free(utf8);

    /* Group table name */
    text = g_strdup_printf("groups");
    utf8 = g_locale_to_utf8(text, strlen(text), NULL, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(widgets->remote_grouptable_entry), utf8);
    if( text!=NULL )
      g_free(text);
    if( utf8!=NULL )
      g_free(utf8);

    /* Buttons, "Cancel" and "Apply" */
    GtkWidget *hbutton_box = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbutton_box), GTK_BUTTONBOX_SPREAD);

    GtkWidget *cancel_button, *apply_button;
    cancel_button = gtk_button_new_from_stock(GTK_STOCK_QUIT);
    apply_button = gtk_button_new_from_stock(GTK_STOCK_APPLY);

    gtk_box_pack_start(GTK_BOX(hbutton_box), cancel_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbutton_box), apply_button, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(menu_vbox), hbutton_box);

    /* Window exit signal */
    g_signal_connect(GTK_WINDOW(widgets->mysql_window), "delete_event",
                     G_CALLBACK(gtk_widget_destroy), NULL);

    /* Quit / Cancel button */
    g_signal_connect_swapped((gpointer)cancel_button, "clicked",
                                 G_CALLBACK(gtk_widget_destroy),
                               G_OBJECT(widgets->mysql_window));
    /* Apply button */
    g_signal_connect_swapped((gpointer)apply_button, "clicked",
                G_CALLBACK(mysql_window_apply_clicked), widgets);

    gtk_widget_show_all(widgets->mysql_window);
}
