AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4 --install

bin_PROGRAMS=
lib_LTLIBRARIES=

CLEANFILES=
CFLAGS=-g
CXXFLAGS=-g -fpermissive
export DYLD_FORCE_FLAT_NAMESPACE=1

LIBTAP=deps/libtap++-0.01/libtap++.la

$(LIBTAP): deps/libtap++-0.01/tap++.C
deps/libtap++-0.01/tap++.C:
	cd deps && tar xzf libtap-0.01.tar.gz
# need bogus -rpath to force shared library
deps_libtap___0_01_libtap___la_LDFLAGS = -rpath /dev/null
deps_libtap___0_01_libtap___la_SOURCES = \
	libtap++-0.01/tap++.h \
	deps/libtap++-0.01/tap++.C

if WITH_PHP
test-more-php/Test-More-OO.php: deps/Test-More-PHP-0.2.tar.gz
	cd deps && tar xzf Test-More-PHP-0.2.tar.gz

check_HEADERS = test-more-php/Test-More-OO.php
endif
check_LTLIBRARIES = $(LIBTAP)
check_PROGRAMS = 
TESTS =

TestMakefile.am: auto/testAutoMake.sh
	auto/testAutoMake.sh > TestMakefile.am
include TestMakefile.am

###############################################
### gearbox/core
###############################################
bin_PROGRAMS+=gearbox/core/gearbox_json
lib_LTLIBRARIES+=gearbox/core/libgearbox_core.la

gearbox_core_libgearbox_core_la_CXXFLAGS = $(YAJL_CFLAGS) $(CURL_CFLAGS) $(BOOST_CPPFLAGS) $(LOG4CXX_CFLAGS) $(UUID_CFLAGS)
gearbox_core_libgearbox_core_la_LDFLAGS = $(YAJL_LIBS) $(CURL_LIBS) $(BOOST_LDFLAGS) $(BOOST_FILESYSTEM_LIB) $(LOG4CXX_LIBS) $(BOOST_REGEX_LIB) $(CRYPTO_LIB) $(UUID_LIBS) $(BOOST_IOSTREAMS_LIB)
gearbox_core_libgearbox_core_includes = \
	gearbox/core/ConfigFile.h \
	gearbox/core/Errors.h \
	gearbox/core/File.h \
	gearbox/core/Hash.h \
	gearbox/core/HttpClient.h \
	gearbox/core/Json.h \
    gearbox/core/JsonGenerator.h \
    gearbox/core/JsonParser.h \
    gearbox/core/JsonPrivate.h \
    gearbox/core/JsonSchema.h \
    gearbox/core/logger.h \
    gearbox/core/Pipe.h \
    gearbox/core/Plugin.h \
    gearbox/core/REST.h \
    gearbox/core/strlcpy.h \
    gearbox/core/SystemPair.h \
    gearbox/core/TempDir.h \
    gearbox/core/TempFile.h \
    gearbox/core/Uri.h \
    gearbox/core/util.h \
	$(NULL)

gearbox_core_libgearbox_core_la_SOURCES = $(gearbox_core_libgearbox_core_includes) \
    gearbox/core/ConfigFile.cc \
    gearbox/core/Errors.cc \
	gearbox/core/ffdigest.cc \
    gearbox/core/File.cc \
    gearbox/core/Hash.cc \
    gearbox/core/HttpClient.cc \
    gearbox/core/Json.cc \
    gearbox/core/JsonGenerator.cc \
    gearbox/core/JsonParser.cc \
    gearbox/core/JsonSchema.cc \
    gearbox/core/logger.cc \
    gearbox/core/Pipe.cc \
    gearbox/core/Plugin.cc \
    gearbox/core/REST.cc \
    gearbox/core/run.cc \
    gearbox/core/sha1.cc \
    gearbox/core/SystemPair.cc \
    gearbox/core/TempDir.cc \
    gearbox/core/TempFile.cc \
    gearbox/core/Uri.cc \
    gearbox/core/util.cc \
    $(NULL)

gearbox_core_gearbox_json_SOURCES = gearbox/core/gearbox_json.cc
gearbox_core_gearbox_json_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB)
gearbox_core_gearbox_json_LDADD = gearbox/core/libgearbox_core.la

###############################################
### gearbox/job
###############################################
lib_LTLIBRARIES+=gearbox/job/libgearbox_job.la

gearbox_job_libgearbox_job_la_CXXFLAGS = $(GEARMAN_CFLAGS)
gearbox_job_libgearbox_job_la_LDFLAGS = $(GEARMAN_LIBS) $(BOOST_LDFLAGS) $(BOOST_GRAPH_LIB)
gearbox_job_libgearbox_job_la_LIBADD = gearbox/core/libgearbox_core.la
gearbox_job_libgearbox_job_includes = \
    gearbox/job/GearmanJobImpl.h \
    gearbox/job/Job.h \
    gearbox/job/JobImpl.h \
    gearbox/job/JobManager.h \
    gearbox/job/JobResponse.h \
    gearbox/job/JobStatusImpl.h \
    gearbox/job/JsonStatusImpl.h \
    gearbox/job/RestJobImpl.h \
    gearbox/job/Status.h \
    gearbox/job/StatusCollection.h \
    gearbox/job/StatusCollectionImpl.h \
    gearbox/job/StatusCollectionImplV1.h \
    gearbox/job/StatusImpl.h \
    gearbox/job/StatusImplV1.h \
    gearbox/job/StatusManager.h \
    gearbox/job/TransientStatusImpl.h \
    $(NULL)

gearbox_job_libgearbox_job_la_SOURCES = $(gearbox_job_libgearbox_job_includes) \
    gearbox/job/GearmanJobImpl.cc \
    gearbox/job/Job.cc \
    gearbox/job/JobImpl.cc \
    gearbox/job/JobManager.cc \
    gearbox/job/JobResponse.cc \
    gearbox/job/JobStatusImpl.cc \
    gearbox/job/JsonStatusImpl.cc \
    gearbox/job/RestJobImpl.cc \
    gearbox/job/Status.cc \
    gearbox/job/StatusCollection.cc \
    gearbox/job/StatusCollectionImpl.cc \
    gearbox/job/StatusCollectionImplV1.cc \
    gearbox/job/StatusImpl.cc \
    gearbox/job/StatusImplV1.cc \
    gearbox/job/StatusManager.cc \
    gearbox/job/TransientStatusImpl.cc \
    $(NULL)

###############################################
### gearbox/store
###############################################
lib_LTLIBRARIES+=gearbox/store/libgearbox_store.la

gearbox_store_libgearbox_store_la_CXXFLAGS = 
gearbox_store_libgearbox_store_la_LDFLAGS = $(SOCI_CORE_LIB)
gearbox_store_libgearbox_store_la_LIBADD = gearbox/core/libgearbox_core.la
gearbox_store_libgearbox_store_includes = \
    gearbox/store/dbconn.h \
    $(NULL)

gearbox_store_libgearbox_store_la_SOURCES = $(gearbox_store_libgearbox_store_includes) \
	gearbox/store/dbconn.cc \
	gearbox/store/LoggedStatement.cc \
    $(NULL)

###############################################
### gearbox/worker
###############################################
lib_LTLIBRARIES+=gearbox/worker/libgearbox_worker.la

gearbox_worker_libgearbox_worker_la_CXXFLAGS = 
gearbox_worker_libgearbox_worker_la_LDFLAGS = $(SOCI_CORE_LIB)
gearbox_worker_libgearbox_worker_la_LIBADD = gearbox/core/libgearbox_core.la gearbox/store/libgearbox_store.la gearbox/job/libgearbox_job.la
gearbox_worker_libgearbox_worker_includes = \
    gearbox/worker/Worker.h \
    $(NULL)

gearbox_worker_libgearbox_worker_la_SOURCES = $(gearbox_worker_libgearbox_worker_includes) \
	gearbox/worker/Worker.cc \
    $(NULL)


############################################################
### workers/gearbox - workers for internal gearbox services
############################################################
bin_PROGRAMS+=workers/gearbox/workerGearbox
bin_PROGRAMS+=workers/gearbox/delayDaemon

lib_LTLIBRARIES+=workers/gearbox/libworker_gearbox.la

workers_gearbox_libworker_gearbox_includes = \
	workers/gearbox/WorkerGearbox.h \
	workers/gearbox/DelayProcessor.h \
	$(NULL)

workers_gearbox_libworker_gearbox_la_SOURCES = \
	$(workers_gearbox_libworker_gearbox_includes) \
	workers/gearbox/WorkerGearbox.cc \
    workers/gearbox/DelayProcessor.cc \
	$(NULL)
workers_gearbox_libworker_gearbox_la_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_FILESYSTEM_LIB)
workers_gearbox_libworker_gearbox_la_LIBADD = gearbox/core/libgearbox_core.la gearbox/store/libgearbox_store.la gearbox/worker/libgearbox_worker.la

workers_gearbox_workerGearbox_SOURCES = workers/gearbox/workerGearboxMain.cc
workers_gearbox_workerGearbox_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB)
workers_gearbox_workerGearbox_LDADD = gearbox/core/libgearbox_core.la gearbox/store/libgearbox_store.la workers/gearbox/libworker_gearbox.la

workers_gearbox_delayDaemon_SOURCES = workers/gearbox/delayDaemon.cc
workers_gearbox_delayDaemon_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB)
workers_gearbox_delayDaemon_LDADD = workers/gearbox/libworker_gearbox.la

###############################################
### gearbox/swig/perl
###############################################
lib_LTLIBRARIES+=gearbox/swig/perl/libGearbox.la
perllibdir=${PERL_SITELIBDIR}
perllib_HEADERS = gearbox/swig/perl/lib/Gearbox/Errors.pm

gearbox/swig/perl/lib/Gearbox/Errors.pm: gearbox/core/Errors.h
	gearbox/swig/perl/genErrors > gearbox/swig/perl/lib/Gearbox/Errors.pm

gearbox_swig_perl_libGearbox_la_CXXFLAGS = $(filter-out $(PERL_ARCH),$(PERL_CFLAGS))
gearbox_swig_perl_libGearbox_la_LDFLAGS = -avoid-version -shared $(filter-out $(PERL_ARCH),$(PERL_LDFLAGS))
gearbox_swig_perl_libGearbox_la_LIBADD = gearbox/core/libgearbox_core.la gearbox/job/libgearbox_job.la gearbox/worker/libgearbox_worker.la
gearbox_swig_perl_libGearbox_la_includes = \
    gearbox/swig/perl/SwigGearbox_wrap.h \
	gearbox/swig/SwigWorker.h \
    $(NULL)

gearbox_swig_perl_libGearbox_la_SOURCES = $(gearbox_swig_perl_libGearbox_includes) \
	gearbox/swig/perl/SwigGearbox_wrap.cc \
	gearbox/swig/SwigWorker.cc \
    $(NULL)

gearbox/swig/perl/SwigGearbox_wrap.cc gearbox/swig/perl/SwigGearbox_wrap.h: gearbox/swig/SwigGearbox.i gearbox/swig/perl/perl.i gearbox/swig/perl/perlworker.i
	$(SWIG) -perl -c++ -o gearbox/swig/perl/SwigGearbox_wrap.cc -outdir gearbox/swig/perl/lib -I$(abs_top_srcdir) gearbox/swig/SwigGearbox.i

CLEANFILES += \
	gearbox/swig/perl/lib/Gearbox.pm \
	gearbox/swig/perl/lib/Gearbox/Errors.pm \
	gearbox/swig/perl/SwigGearbox_wrap.cc \
	gearbox/swig/perl/SwigGearbox_wrap.h \
	$(NULL)

if WITH_PHP
###############################################
### gearbox/swig/php
###############################################
lib_LTLIBRARIES+=gearbox/swig/php/SwigGearbox.la

gearbox_swig_php_SwigGearbox_la_CXXFLAGS = $(PHP_CFLAGS)
gearbox_swig_php_SwigGearbox_la_LDFLAGS = -module -avoid-version -shared $(PHP_LDFLAGS)
gearbox_swig_php_SwigGearbox_la_LIBADD = gearbox/core/libgearbox_core.la gearbox/job/libgearbox_job.la gearbox/worker/libgearbox_worker.la
gearbox_swig_php_SwigGearbox_la_includes = \
    gearbox/swig/php/php_SwigGearbox.h \
	gearbox/swig/SwigGearbox_wrap.h \
	gearbox/swig/SwigWorker.h \
    $(NULL)

gearbox_swig_php_SwigGearbox_la_SOURCES = $(gearbox_swig_php_libGearbox_includes) \
	gearbox/swig/php/SwigGearbox_wrap.cc \
	gearbox/swig/SwigWorker.cc \
    $(NULL)

gearbox/swig/php/php_SwigGearbox.h gearbox/swig/php/SwigGearbox.php gearbox/swig/php/SwigGearbox_wrap.cc gearbox/swig/php/SwigGearbox_wrap.h: gearbox/swig/SwigGearbox.i gearbox/swig/php/php.i gearbox/swig/php/phpworker.i
	$(SWIG) -php -cppext cxx -c++ -prefix "Gearbox" -o gearbox/swig/php/SwigGearbox_wrap.cc -outdir gearbox/swig/php -I$(abs_top_srcdir) gearbox/swig/SwigGearbox.i
	perl -pi -e 's/(SWIG_SetPointerZval\(.*?),\s+0\)/$$1, 2)/g' gearbox/swig/php/SwigGearbox_wrap.cc
	perl -pi -e 's/Gearbox__//g' gearbox/swig/php/SwigGearbox_wrap.cc gearbox/swig/php/SwigGearbox.php
	perl -pi -e 's/new RealJobManager/new JobManager/g' gearbox/swig/php/SwigGearbox.php
	echo "<?php" > gearbox/swig/php/GearboxErrors.php
	echo 'class GearboxError extends Exception {\n  function getName() { return $$this->name; }\n}\n' >> gearbox/swig/php/GearboxErrors.php
	cat gearbox/core/Errors.h | $(PERL_BIN) -ne 's/DEFINE_ERROR\((\w+),(\d+)\);/class ERR_$$1 extends GearboxError {\n  protected \$$name = "$$1";\n  public function __construct(\$$message) {\n    parent::__construct("ERR_$$1: ".\$$message,$$2);\n  }\n}\n\nclass ERR_CODE_$$2 extends ERR_$$1 {}\n/ && print' >> gearbox/swig/php/GearboxErrors.php
	echo "?>" >> gearbox/swig/php/GearboxErrors.php

CLEANFILES += \
	gearbox/swig/php/SwigGearbox.php \
	gearbox/swig/php/GearboxErrors.php \
	gearbox/swig/php/SwigGearbox_wrap.cc \
	gearbox/swig/php/SwigGearbox_wrap.h \
	gearbox/swig/php/php_SwigGearbox.h \
	$(NULL)
endif

