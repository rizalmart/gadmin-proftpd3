## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

sbin_PROGRAMS = gadmin-proftpd gprostats

gadmin_proftpd_SOURCES = \
	gadmin_proftpd.c gadmin_proftpd.h \
	populate_gadmin_proftpd.c populate_gadmin_proftpd.h \
	widgets.c widgets.h \
	support.c support.h \
	create_main_window.c create_main_window.h \
	create_server_tab.c create_server_tab.h \
	populate_servers.c populate_servers.h \
	server_treeview_row_clicked.c server_treeview_row_clicked.h \
	add_server.c add_server.h \
	delete_server.c delete_server.h \
	create_server_settings.c create_server_settings.h \
	populate_server_settings.c populate_server_settings.h \
	apply_server_settings.c apply_server_settings.h \
	select_first_server.c select_first_server.h \
	create_user_tab.c create_user_tab.h \
	file_chooser.c file_chooser.h \
	populate_users.c populate_users.h \
	user_treeview_row_clicked.c user_treeview_row_clicked.h \
	populate_shell_combo.c populate_shell_combo.h \
	add_user.c add_user.h \
	dir_treeview_funcs.c dir_treeview_funcs.h \
	create_user_settings.c create_user_settings.h \
	populate_user_settings.c populate_user_settings.h \
	select_first_user.c select_first_user.h \
	apply_user.c apply_user.h \
	delete_user.c delete_user.h \
	create_del_system_user_question.c create_del_system_user_question.h \
	make_settings_entries.c make_settings_entries.h \
	make_settings_combos.c make_settings_combos.h \
	make_settings_spinbuttons.c make_settings_spinbuttons.h \
	make_settings_buttons.c make_settings_buttons.h \
	make_settings_checkbuttons.c make_settings_checkbuttons.h \
	make_settings_progressbars.c make_settings_progressbars.h \
	make_settings_labels.c make_settings_labels.h \
	make_settings_hseparators.c make_settings_hseparators.h \
	create_transfer_tab.c create_transfer_tab.h \
	kick_button_clicked.c kick_button_clicked.h \
	kick_ban_button_clicked.c kick_ban_button_clicked.h \
	status_update.c status_update.h \
	create_disc_tab.c create_disc_tab.h \
	populate_disc_tab.c populate_disc_tab.h \
	create_file_tab.c create_file_tab.h \
	populate_file_tab.c populate_file_tab.h \
	clear_file_tab.c clear_file_tab.h \
	create_security_tab.c create_security_tab.h \
	populate_security_tab.c populate_security_tab.h \
	clear_security_tab.c clear_security_tab.h \
	create_conf_tab.c create_conf_tab.h \
	populate_conf_tab.c populate_conf_tab.h \
	save_conf_tab.c save_conf_tab.h \
	set_version.c set_version.h \
	reread_conf.c reread_conf.h \
	apply_button_clicked.c apply_button_clicked.h \
	commands.c commands.h \
	generate_cert.c generate_cert.h \
	functions.c functions.h \
	mysql_functions.c mysql_functions.h \
	user_auth_directives.c user_auth_directives.h \
	process_window.c process_window.h \
	credits_window.c credits_window.h \
	activate_button_clicked.c activate_button_clicked.h \
	deactivate_button_clicked.c deactivate_button_clicked.h \
	show_info.c show_info.h \
	allocate.c allocate.h \
	system_defines.c system_defines.h \
	create_import_window.c create_import_window.h \
	import_functions.c import_functions.h \
	show_shutdown.c show_shutdown.h \
	shutdown_ok_clicked.c shutdown_ok_clicked.h \
	standard_conf.c standard_conf.h \
	add_standard_users.c add_standard_users.h \
	show_help.c show_help.h

gprostats_SOURCES = \
	gprostats.c \
	allocate.c allocate.h

gadmin_proftpd_LDADD = @PACKAGE_LIBS@

