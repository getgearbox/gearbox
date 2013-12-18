%define destdir %buildroot
%define cmddir /command
%define srvdir /service

Name: daemontools
Version: 0.76
Release: 1%{?dist}
Summary: daemontools is a collection of tools for managing UNIX services.
Group: Utilities/System
License: Public Domain
Source: http://cr.yp.to/daemontools/daemontools-0.76.tar.gz
BuildRequires: gcc

%description
daemontools is a collection of tools for managing UNIX services.

supervise monitors a service. It starts the service and restarts the
service if it dies. Setting up a new service is easy: all supervise
needs is a directory with a run script that runs the service.

multilog saves error messages to one or more logs. It optionally
timestamps each line and, for each log, includes or excludes lines
matching specified patterns. It automatically rotates logs to limit
the amount of disk space used. If the disk fills up, it pauses and
tries again, without losing any data.

%prep
rm -rf %destdir
%setup -q -c -n %destdir/package
cd admin/%name-%version

# see "errno" section on http://cr.yp.to/docs/unixport.html
sed -i '1s/$/ -include \/usr\/include\/errno.h/' src/conf-cc

%build
cd admin/%name-%version
echo "%{_prefix}" >src/conf-home
package/compile

%install
cd admin/%name-%version
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{cmddir}
mkdir -p %{buildroot}%{srvdir}

install -m 755 command/* %{buildroot}%{_bindir}
for i in `cat package/commands`
do
  ln -s ..%{_bindir}/$i %{buildroot}/command/$i
done

mkdir -p %{buildroot}%{_sysconfdir}/init
cat > %{buildroot}%{_sysconfdir}/init/daemontools.conf <<DAEMONTOOLSINIT
start on runlevel [2345]
stop on runlevel [06]

respawn
exec %{_bindir}/svscanboot
DAEMONTOOLSINIT

# so rpm doesn't complain about files that aren't packaged
rm -rf %{buildroot}/package/admin/daemontools-0.76/{src,compile,command,package}/

%clean

%post
start daemontools

%preun
stop daemontools

%files
%defattr (0775,root,root)
%{_bindir}/svscan
%{_bindir}/svscanboot
%{_bindir}/supervise
%{_bindir}/svc
%{_bindir}/svok
%{_bindir}/svstat
%{_bindir}/fghack
%{_bindir}/multilog
%{_bindir}/pgrphack
%{_bindir}/tai64n
%{_bindir}/tai64nlocal
%{_bindir}/readproctitle
%{_bindir}/softlimit
%{_bindir}/envuidgid
%{_bindir}/envdir
%{_bindir}/setlock
%{_bindir}/setuidgid
%{_sysconfdir}/init/daemontools.conf
%dir %{cmddir}
%{cmddir}/*
%dir %{srvdir}

%changelog
* Wed Dec 18 2013 Jay Buffington <me@jaybuff.com> - 0.76-1
  - initial package
