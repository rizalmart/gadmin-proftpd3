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



#ifndef create_user_tab_H
#   define create_user_tab_H

extern struct w *widgets;

void create_user_tab(struct w *widgets);

void directory_cell_edited(GtkCellRendererText *cell,
			   gchar *path_string,
			   gchar *new_text,
			   struct w *widgets);

void toggle_button_1_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_2_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_3_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_4_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_5_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_6_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_7_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_8_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_9_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_10_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_11_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_12_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_13_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_14_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_15_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_16_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_17_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);
void toggle_button_18_clicked(GtkCellRendererToggle *cell_renderer, gchar *path, struct w *widgets);

void add_directory(gchar *dir, struct w *widgets);
void select_directory_clicked(struct w *widgets);
void del_directory(struct w *widgets);

#endif
