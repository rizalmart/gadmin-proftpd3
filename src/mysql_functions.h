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

#ifndef mysql_functions_H
#define mysql_functions_H

extern struct w *widgets;

void show_mysql_window(struct w *widgets);

void mysql_window_apply_clicked(struct w *widgets);
void mysql_window_cancel_clicked(struct w *widgets);

char ** mysql_get_connect_args();
void mysql_free_connect_args(char **connect_args);

int mysql_connect(char **connect_args,
                  struct w *widgets, int cmd_num, char **user_args);

int mysql_run_process_cmd(char **new_argv, char **connect_args,
                          struct w *widgets, int cmd_num, char **user_args);

int mysql_handle_output(int fd, char **connect_args,
                        struct w *widgets, int *cmd_num, char **user_args);

int mysql_create_database(int fd, gchar *dbname);
int mysql_create_tables(int fd, gchar *dbname, gchar *usertable, gchar *grouptable);

int mysql_add_user(int fd, struct w *widgets, char **connect_args, char **user_args);
int mysql_delete_user(int fd, struct w *widgets, char **connect_args, char **user_args);

int mysql_add_group(int fd, struct w *widgets,
                    char **connect_args, char **user_args, char *gid);
int mysql_delete_group(int fd, struct w *widgets, char **connect_args, char **user_args);

int mysql_disconnect(int fd);

int write_password(int fd, gchar *pass);
int write_string(int fd, char *data);

#endif
