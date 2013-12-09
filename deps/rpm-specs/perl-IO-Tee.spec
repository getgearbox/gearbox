Name:           perl-IO-Tee
Version:        0.64
Release:        1
Summary:        Multiplex output to multiple output handles 
License:        Artistic
Group:          Development/Libraries
URL:            http://search.cpan.org/~kenshan/IO-Tee/
Source0:        http://search.cpan.org/CPAN/authors/id/K/KE/KENSHAN/IO-Tee-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:      noarch
BuildRequires:  perl(ExtUtils::MakeMaker)

%description
The `IO::Tee' constructor, given a list of output handles, returns a
tied handle that can be written to but not read from. When written to
(using print or printf), it multiplexes the output to the list of
handles originally passed to the constructor. As a shortcut, you can
also directly pass a string or an array reference to the constructor, in
which case `IO::File::new' is called for you with the specified argument
or arguments.


%prep
%setup -q -n IO-Tee-%{version}

%build
%{__perl} Makefile.PL INSTALLDIRS=vendor < /dev/null
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT

make pure_install DESTDIR=$RPM_BUILD_ROOT

find $RPM_BUILD_ROOT -type f -name .packlist -exec rm -f {} \;
find $RPM_BUILD_ROOT -depth -type d -exec rmdir {} 2>/dev/null \;

%{_fixperms} $RPM_BUILD_ROOT/*

%check
make test

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc Changes README
%{perl_vendorlib}/IO*
%{_mandir}/man3/IO*.3*

%changelog
* Fri Dec  6 2013 Jay Buffington <me@jaybuff.com> - 0.64-1
- initial package
