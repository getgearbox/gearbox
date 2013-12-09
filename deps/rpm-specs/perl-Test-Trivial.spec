Name:           perl-Test-Trivial
Version:        1.901.2
Release:        1
Summary:        Test::Trivial - Declutter and simplify tests
License:        Artistic
Group:          Development/Libraries
URL:            https://github.com/coryb/perl-test-trivial
Source0:        http://www.cpan.org/authors/id/C/CO/CORYB/Test-Trivial-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:      noarch
BuildRequires:  perl(ExtUtils::MakeMaker) perl(Regexp::Common) perl(Text::Diff) perl(Test::More) perl(IO::Scalar) perl(IO::Tee)

%description
"Test::Trivial" was written to allow test writers to trivially write
tests while still allowing the test code to be readable. The output upon
failure has been modified to provide better diagnostics when things go
wrong, including the source line number for the failed test. Global
getopt options are automatically added to all tests files to allow for
easier debugging when things go wrong.

%prep
%setup -q -n Test-Trivial-%{version}

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
%doc CHANGES README LICENSE.TXT
%{perl_vendorlib}/Test*
%{_mandir}/man3/Test*.3*

%changelog
* Fri Dec  6 2013 Jay Buffington <me@jaybuff.com> - 1.901.2-1
- initial package
