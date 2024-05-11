%define desktop_vendor newrpms
%{!?_dist: %{expand: %%define dist rhfc16}}

Summary:	GADMIN-ProFTPD -- A GTK+ administation tool for the ProFTPD server.
Name:		gadmin-proftpd
Version:	0.4.6
Release:	0.1.%{dist}.nr
License:	GPL
Group:		Applications/System
URL:		http://dalalven.dtdns.net/linux.html
Source0:	http://dalalven.dtdns.net/linux/%{name}/%{name}-%{version}.tar.gz
BuildRoot:	%{_builddir}/%{name}-%{version}-root
Provides:	gadmin-proftpd gprostats

%description
GADMIN-ProFTPD is a fast and easy to use GTK+ administration tool for the proftpd standalone server.

%prep
%setup -q
%configure SYSINIT_START_CMD="chkconfig proftpd on" SYSINIT_STOP_CMD="chkconfig proftpd off"

%build
%{__make}

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall INSTALL_USER=`id -un` INSTALL_GROUP=`id -gn`

# pam auth
install -d %{buildroot}%{_sysconfdir}/pam.d/
install -d %{buildroot}%{_sysconfdir}/security/console.apps
install -m 644 etc/pam.d/%{name} %{buildroot}%{_sysconfdir}/pam.d/%{name}
install -m 644 etc/security/console.apps/%{name} %{buildroot}%{_sysconfdir}/security/console.apps/%{name}

# desktop entry
install -d %{buildroot}%{_datadir}/applications
install -m 644 desktop/gadmin-proftpd.desktop %{buildroot}%{_datadir}/applications/%{name}.desktop

# doc files.. 
install -d %{buildroot}%{_datadir}/doc/%{name}
install -m 644  README COPYING AUTHORS ChangeLog %{buildroot}%{_datadir}/doc/%{name}

%find_lang %name

%post 
if test ! -h %{_bindir}/gadmin-proftpd ; then \
ln -s %{_bindir}/consolehelper %{_bindir}/gadmin-proftpd ; \
fi;

%clean
rm -rf $RPM_BUILD_ROOT

%files -f %{name}.lang
%defattr(0755, root, root)
%{_sbindir}/%{name}
%{_sbindir}/gprostats

%defattr(0644, root, root)
#%doc COPYING AUTHORS ChangeLog
%{_datadir}/doc/%{name}/README
%{_datadir}/doc/%{name}/COPYING 
%{_datadir}/doc/%{name}/AUTHORS 
%{_datadir}/doc/%{name}/ChangeLog

%{_sysconfdir}/pam.d/%{name}
%{_sysconfdir}/security/console.apps/%{name}

%{_datadir}/applications/%{name}.desktop
%{_datadir}/pixmaps/*.png

%dir %{_datadir}/pixmaps/%{name}
%{_datadir}/pixmaps/%{name}/*.png
%{_datadir}/pixmaps/%{name}/%{name}36.xpm

%changelog
* Sun Jun 2 2013 Magnus-swe <magnus-swe@telia.com>
- Fixed an iter validation issue and updated
- the code for showing file transfers.
* Tue Jan 9 2013 Magnus-swe <magnus-swe@telia.com>
- Authentication types are now selectable with combo choices.
- ModulePath is added if found and missing.
- Adds MySQL-support via MySQL interprocess communication.
- MySQL database setup adds user, group and quota tables.
- Remote and local MySQL users can be added, changed and deleted.
* Tue Dec 7 2011 Magnus-swe <magnus-swe@telia.com>
- Indentation fixes and cleanups.
- Fixes a possible first time startup crash.
* Sat May 21 2011 Magnus-swe <magnus-swe@telia.com>
- Dont add ratio directives if the ratio module is loaded
- but ratios arent turned on in the global settings.
* Sat Jan 15 2011 Magnus-swe <magnus-swe@telia.com>
- Module detection improvement.
* Wed Mar 17 2010 Magnus-swe <magnus-swe@telia.com>
- Better handling of directories with spaces and
- the AllowOverwrite option.
* Thu Feb 11 2010 Magnus-swe <magnus-swe@telia.com>
- Adds TLS option:
- TLSOptions AllowClientRenegotiations
* Fri Aug 28 2009 Magnus-swe <magnus-swe@telia.com>
- Removes the global apply and directory apply buttons.
- Combines "add user" and "apply user" buttons. 
- Adds more information about adding new users.
- Sets missing version color.
- Swedish translation update.
* Wed Jul 22 2009 Magnus-swe <magnus-swe@telia.com>
- Adds a new user button.
- Status is shown in red or green.
* Sat Jun 20 2009 Magnus-swe <magnus-swe@telia.com>
- Adds support for dynamically loaded modules.
* Thu Oct 15 2008 Magnus-swe <magnus-swe@telia.com>
- Adds a new Swedish translation.
- Configuration backup, restore and auto backup.
* Thu Oct 09 2008 Magnus-swe <magnus-swe@telia.com>
- Fixed the way it populates and mod_tls handling.
* Thu Sep 25 2008 Magnus-swe <magnus-swe@telia.com>
- Fixed certificates validity days.
* Wed Jun 18 2008 Magnus-swe <magnus-swe@telia.com>
- Added signed certificate creation and explicit encryption.
