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

/* Make sure we have enough of these or it will
   borrow from the next widget-type and fail */
#define NUM_SERVERTAB_ENTRIES 25
#define NUM_SERVERTAB_SPINBUTTONS 13
#define NUM_SERVERTAB_COMBOS 19


#define NUM_USERTAB_ENTRIES 7
#define NUM_USERTAB_SPINBUTTONS 5
#define NUM_USERTAB_CHECKBUTTONS 3
#define NUM_USERTAB_COMBOS 1


typedef struct w
{
    GtkWidget *main_window;
    GtkWidget *main_vbox;

    GtkWidget *notebook_vbox1;
    GtkWidget *notebook_vbox2;
    GtkWidget *notebook_vbox3;
    GtkWidget *notebook_vbox4;
    GtkWidget *notebook_vbox5;
    GtkWidget *notebook_vbox6;
    GtkWidget *notebook_vbox7;

    GtkWidget *status_label;
    GtkWidget *version_label;

    /* The server tab's widgets */
    GtkWidget *server_treeview;
    GtkListStore *server_store;
    GtkWidget *server_settings_vbox;
    GtkWidget *server_settings_scrolled_window;

    GtkWidget *server_set_entry[NUM_SERVERTAB_ENTRIES];
    GtkWidget *server_set_spinbutton[NUM_SERVERTAB_SPINBUTTONS];
    GtkWidget *server_set_combo[NUM_SERVERTAB_COMBOS];
    GtkWidget *srv_set_table;

    /* The user tab's widgets */
    GtkWidget *user_treeview;
    GtkListStore *user_store;
    GtkWidget *user_settings_vbox;
    GtkWidget *user_settings_scrolled_window;
    GtkWidget *user_set_entry[NUM_USERTAB_ENTRIES];
    GtkWidget *user_set_spinbutton[NUM_USERTAB_SPINBUTTONS];
    GtkWidget *user_set_checkbutton[NUM_USERTAB_CHECKBUTTONS];
    GtkWidget *user_set_combo[NUM_USERTAB_COMBOS];
    GtkWidget *gen_cert_progressbar;
    GtkWidget *usr_set_table;

    /* The add user widgets */
    GtkWidget *user_button_box;
    GtkWidget *useradd_username_entry;
    GtkWidget *useradd_password_entry;
    GtkWidget *useradd_homedir_entry;
    GtkWidget *useradd_usercomment_entry;
    GtkWidget *useradd_shell_combo;
    GtkWidget *useradd_uploaddir_checkbutton;


    /* The user tab's directory treeview */
    GtkWidget *directory_treeview;
    GtkListStore *directory_store;

    /* The delete system user question window */
    GtkWidget *del_system_user_question_window;

    /* The add default configuration question window */
    GtkWidget *default_conf_question_window;

    /* The transfer tabs widgets */
    GtkWidget *transfer_treeview;
    GtkListStore *transfer_store;
    GtkWidget *total_bandwidth_label;
    GtkWidget *total_incoming_label;
    GtkWidget *total_outgoing_label;
    GtkWidget *kick_entry;
    GtkWidget *transfer_pid_checkbutton;

    /* The disc tabs widgets */
    GtkWidget *disc_treeview;
    GtkListStore *disc_store;

    /* The file treeview */
    GtkWidget *file_treeview;
    GtkListStore *file_store;

    /* The security tabs widgets */
    GtkWidget *security_treeview;
    GtkListStore *security_store;

    /* The conf tabs textview */
    GtkWidget *conf_textview;

    /* The import windows widgets */
    GtkWidget *import_window;
    GtkWidget *import_treeview;
    GtkListStore *import_store;
    GtkWidget *import_home_entry;
    GtkWidget *import_with_username_checkbutton;

    /* The shutdown windows widgets */
    GtkWidget *shutdown_window;
    GtkWidget *new_acc_disabled_entry;
    GtkWidget *existing_users_dc_entry;
    GtkWidget *real_shutdown_entry;
    GtkWidget *shutdown_msg_entry;

    /* Mysql widgets */
    GtkWidget *mysql_window;

    /* Generic database widgets for MySQL and LDAP etc */
    GtkWidget *remote_server_entry;
    GtkWidget *remote_port_entry;
    GtkWidget *remote_user_entry;
    GtkWidget *remote_password_entry;
    GtkWidget *remote_database_entry;

    GtkWidget *remote_usertable_entry;
    GtkWidget *remote_grouptable_entry;

    /* Window for showing database process output */
    GtkWidget *process_window;
    GtkWidget *process_textview;

} wid;
