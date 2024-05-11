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



#ifndef commands_H
#	define commands_H

extern struct w *widgets;

int run_command(gchar *command);
void run_command_show_err(gchar *command);
int file_exists(char *infile);
int using_tls(void);
int using_ratio(void);
int using_quota(void);
void init_start(struct w *widgets);
void init_stop(struct w *widgets);

char * get_dynamic_module_statements();
char * get_included_dynamic_module_statements(char *path);

int using_module(char module_name[1024]);
int using_included_module(char module_name[1024]);
int module_in_include_path(char *path, char module_name[1024]);

#endif
