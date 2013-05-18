#!/bin/bash

PERL_ARCHNAME=$(perl -MConfig -e 'print $Config{archname}')
AM_PERL_ARCHNAME=$(perl -MConfig -e 'print $Config{archname}' | tr [:punct:][:space:] _)

cat <<EOF
lib_LTLIBRARIES+=gearbox/swig/perl/lib/${PERL_ARCHNAME}/libGearbox.la

gearbox_swig_perl_lib_${AM_PERL_ARCHNAME}_libGearbox_la_CXXFLAGS = \$(filter-out \$(PERL_ARCH),\$(PERL_CFLAGS))
gearbox_swig_perl_lib_${AM_PERL_ARCHNAME}_libGearbox_la_LDFLAGS = -avoid-version -shared \$(filter-out \$(PERL_ARCH),\$(PERL_LDFLAGS))
gearbox_swig_perl_lib_${AM_PERL_ARCHNAME}_libGearbox_la_LIBADD = gearbox/core/libgearbox_core.la gearbox/job/libgearbox_job.la gearbox/worker/libgearbox_worker.la
gearbox_swig_perl_lib_${AM_PERL_ARCHNAME}_libGearbox_la_includes = \
    gearbox/swig/perl/SwigGearbox_wrap.h \
	gearbox/swig/SwigWorker.h \
    \$(NULL)

gearbox_swig_perl_lib_${AM_PERL_ARCHNAME}_libGearbox_la_SOURCES = \$(gearbox_swig_perl_lib_${AM_PERL_ARCHNAME}_libGearbox_includes) \
	gearbox/swig/perl/SwigGearbox_wrap.cc \
	gearbox/swig/SwigWorker.cc \
    \$(NULL)

gearbox/swig/perl/SwigGearbox_wrap.cc gearbox/swig/perl/SwigGearbox_wrap.h: gearbox/swig/SwigGearbox.i gearbox/swig/perl/perl.i gearbox/swig/perl/perlworker.i
	\$(SWIG) -perl -c++ -o gearbox/swig/perl/SwigGearbox_wrap.cc -outdir gearbox/swig/perl/lib -I\$(abs_top_srcdir) gearbox/swig/SwigGearbox.i

CLEANFILES = gearbox/swig/perl/SwigGearbox_wrap.cc gearbox/swig/perl/SwigGearbox_wrap.h
EOF