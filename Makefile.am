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
deps/libtap++-0.01/tap++.C deps/libtap++-0.01/tap++.h:
	cd deps && tar xzf libtap-0.01.tar.gz
# need bogus -rpath to force shared library
deps_libtap___0_01_libtap___la_LDFLAGS = -rpath /dev/null
deps_libtap___0_01_libtap___la_SOURCES = \
	deps/libtap++-0.01/tap++.h \
	deps/libtap++-0.01/tap++.C

dist_check_SCRIPTS =
if WITH_PHP
deps/test-more-php/Test-More-OO.php: deps/Test-More-PHP-0.2.tar.gz
	cd deps && tar xzf Test-More-PHP-0.2.tar.gz

dist_check_SCRIPTS += deps/test-more-php/Test-More-OO.php
else
# hack so "make dist" is happy when we disable php
deps/test-more-php/Test-More-OO.php:
	mkdir -p deps/test-more-php/
	touch $@
endif

deps/PyTAP.tar.gz:
	curl -s https://github.com/rjbs/pytap/tarball/48060b13351979c54cf482629ee41cdeafa4933f -L -o deps/PyTAP.tar.gz

deps/PyTAP: deps/PyTAP.tar.gz
	-rm -rf deps/PyTAP
	cd deps && tar xfz PyTAP.tar.gz && mv rjbs-PyTAP-48060b1 PyTAP

dist_check_SCRIPTS += deps/PyTAP

CLEANFILES += \
	deps/PyTAP.tar.gz \
	$(NULL)

clean-local:
	-rm -rf deps/PyTAP/

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
gearbox_core_libgearbox_core_la_LDFLAGS = $(YAJL_LIBS) $(CURL_LIBS) $(BOOST_LDFLAGS) $(BOOST_FILESYSTEM_LIB) $(LOG4CXX_LIBS) $(BOOST_REGEX_LIB) $(CRYPTO_LIB) $(UUID_LIBS) $(BOOST_IOSTREAMS_LIB) -version-info $(GEARBOX_CORE_LIBRARY_VERSION)
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

coreincludedir = ${pkgincludedir}/core
coreinclude_HEADERS = $(gearbox_core_libgearbox_core_includes)

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
gearbox_job_libgearbox_job_la_LDFLAGS = $(GEARMAN_LIBS) $(BOOST_LDFLAGS) $(BOOST_GRAPH_LIB) -version-info $(GEARBOX_JOB_LIBRARY_VERSION)
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

jobincludedir = ${pkgincludedir}/job
jobinclude_HEADERS = $(gearbox_job_libgearbox_job_includes)

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
gearbox_store_libgearbox_store_la_LDFLAGS = $(SOCI_CORE_LIB) -version-info $(GEARBOX_STORE_LIBRARY_VERSION)
gearbox_store_libgearbox_store_la_LIBADD = gearbox/core/libgearbox_core.la
gearbox_store_libgearbox_store_includes = \
    gearbox/store/dbconn.h \
    $(NULL)

storeincludedir = ${pkgincludedir}/store
storeinclude_HEADERS = $(gearbox_store_libgearbox_store_includes)

gearbox_store_libgearbox_store_la_SOURCES = $(gearbox_store_libgearbox_store_includes) \
	gearbox/store/dbconn.cc \
	gearbox/store/LoggedStatement.cc \
    $(NULL)

###############################################
### gearbox/worker
###############################################
lib_LTLIBRARIES+=gearbox/worker/libgearbox_worker.la

gearbox_worker_libgearbox_worker_la_CXXFLAGS =
gearbox_worker_libgearbox_worker_la_LDFLAGS = $(SOCI_CORE_LIB) -version-info $(GEARBOX_WORKER_LIBRARY_VERSION)
gearbox_worker_libgearbox_worker_la_LIBADD = gearbox/core/libgearbox_core.la gearbox/store/libgearbox_store.la gearbox/job/libgearbox_job.la
gearbox_worker_libgearbox_worker_includes = \
    gearbox/worker/Worker.h \
    $(NULL)

workerincludedir = ${pkgincludedir}/worker
workerinclude_HEADERS = $(gearbox_worker_libgearbox_worker_includes)

gearbox_worker_libgearbox_worker_la_SOURCES = $(gearbox_worker_libgearbox_worker_includes) \
	gearbox/worker/Worker.cc \
    $(NULL)

############################################################
### workers/gearbox - workers for internal gearbox services
############################################################

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

bin_PROGRAMS+=workers/gearbox/workerGearbox
workers_gearbox_workerGearbox_SOURCES = workers/gearbox/workerGearboxMain.cc
workers_gearbox_workerGearbox_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_SYSTEM_LIB)
workers_gearbox_workerGearbox_LDADD = gearbox/core/libgearbox_core.la gearbox/store/libgearbox_store.la workers/gearbox/libworker_gearbox.la

bin_PROGRAMS+=workers/gearbox/delayDaemon
workers_gearbox_delayDaemon_SOURCES = workers/gearbox/delayDaemon.cc
workers_gearbox_delayDaemon_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_SYSTEM_LIB)
workers_gearbox_delayDaemon_LDADD = workers/gearbox/libworker_gearbox.la

schemapkgdatadir = ${pkgdatadir}/schemas/
dist_schemapkgdata_DATA = \
	workers/gearbox/schemas/create-delay-job-v1.js \
	workers/gearbox/schemas/create-global-status-v1.js \
	workers/gearbox/schemas/get-global-status-response-v1.js \
	workers/gearbox/schemas/get-global-status-v1.js \
	workers/gearbox/schemas/update-global-status-v1.js \
	$(NULL)
	
# configuration for daemons
workers/gearbox/conf/config.d/%.conf : workers/gearbox/conf/config.d/%.conf.sh
	$< > $@

workers/gearbox/conf/%.conf : workers/gearbox/conf/%.conf.sh
	$< > $@

gbsysconfdir = ${sysconfdir}/gearbox/
dist_gbsysconf_DATA = \
	workers/gearbox/conf/gearbox.conf \
	$(NULL)

gbconfloggerdir = ${sysconfdir}/gearbox/logger
dist_gbconflogger_DATA = workers/gearbox/conf/logger/gearbox-logger.conf

gbconfigddir = ${gbsysconfdir}/config.d
dist_gbconfigd_DATA = \
	workers/gearbox/conf/config.d/delay.conf \
	workers/gearbox/conf/config.d/gearbox.conf \
	workers/gearbox/conf/config.d/gearman.conf \
	workers/gearbox/conf/config.d/log.conf \
	workers/gearbox/conf/config.d/status.conf \
	$(NULL)

# $< > $@ means execute the source into the target
# e.g. workers/gearbox/sql/status.sql.sh > workers/gearbox/sql/status.sql
workers/gearbox/sql/%.sql : workers/gearbox/sql/%.sql.sh
	$< > $@

workers/gearbox/sql/%-migration/%.sql :  workers/gearbox/sql/%-migration/%.sql.sh
	$< > $@

gbsqldir = ${pkgdatadir}/gearbox_daemons/
dist_gbsql_DATA = \
	workers/gearbox/sql/status.sql \
	workers/gearbox/sql/delay.sql

gbsqlstatusmigratedir = ${gbsqldir}/status-migration
dist_gbsqlstatusmigrate_DATA = \
	workers/gearbox/sql/status-migration/2.sql \
	workers/gearbox/sql/status-migration/2.undo.sql \
	workers/gearbox/sql/status-migration/3.sql \
	workers/gearbox/sql/status-migration/3.undo.sql \
	workers/gearbox/sql/status-migration/4.sql \
	workers/gearbox/sql/status-migration/4.undo.sql \
	workers/gearbox/sql/status-migration/5.sql \
	workers/gearbox/sql/status-migration/5.undo.sql \
	workers/gearbox/sql/status-migration/6.sql \
	workers/gearbox/sql/status-migration/6.undo.sql \
	workers/gearbox/sql/status-migration/7.sql \
	workers/gearbox/sql/status-migration/7.undo.sql \
	workers/gearbox/sql/status-migration/8.sql \
	workers/gearbox/sql/status-migration/8.undo.sql \
	workers/gearbox/sql/status-migration/9.sql \
	workers/gearbox/sql/status-migration/9.undo.sql \
	workers/gearbox/sql/status-migration/10.sql \
	workers/gearbox/sql/status-migration/10.undo.sql \
	workers/gearbox/sql/status-migration/11.sql \
	workers/gearbox/sql/status-migration/11.undo.sql \
	$(NULL)

gbsqldelaymigratedir = ${gbsqldir}/delay-migration
dist_gbsqldelaymigrate_DATA = \
	workers/gearbox/sql/delay-migration/2.sql \
	workers/gearbox/sql/delay-migration/2.undo.sql \
	workers/gearbox/sql/delay-migration/3.sql \
	workers/gearbox/sql/delay-migration/3.undo.sql \
	workers/gearbox/sql/delay-migration/4.sql \
	workers/gearbox/sql/delay-migration/4.undo.sql \
	workers/gearbox/sql/delay-migration/5.sql \
	workers/gearbox/sql/delay-migration/5.undo.sql \
	workers/gearbox/sql/delay-migration/6.sql \
	workers/gearbox/sql/delay-migration/6.undo.sql \
	$(NULL)

CLEANFILES += \
	workers/gearbox/conf/config.d/delay.conf \
	workers/gearbox/conf/config.d/gearbox.conf \
	workers/gearbox/conf/config.d/gearman.conf \
	workers/gearbox/conf/config.d/log.conf \
	workers/gearbox/conf/config.d/status.conf \
	workers/gearbox/conf/gearbox.conf \
	workers/gearbox/sql/delay.sql \
	workers/gearbox/sql/status.sql \
	workers/gearbox/sql/delay-migration/3.undo.sql \
	workers/gearbox/sql/delay-migration/4.sql \
	workers/gearbox/sql/delay-migration/4.undo.sql \
	workers/gearbox/sql/delay-migration/5.sql \
	workers/gearbox/sql/delay-migration/5.undo.sql \
	workers/gearbox/sql/delay-migration/6.sql \
	workers/gearbox/sql/delay-migration/6.undo.sql \
	workers/gearbox/sql/status-migration/10.sql \
	workers/gearbox/sql/status-migration/10.undo.sql \
	workers/gearbox/sql/status-migration/11.sql \
	workers/gearbox/sql/status-migration/11.undo.sql \
	workers/gearbox/sql/status-migration/3.sql \
	workers/gearbox/sql/status-migration/3.undo.sql \
	workers/gearbox/sql/status-migration/4.sql \
	workers/gearbox/sql/status-migration/5.sql \
	workers/gearbox/sql/status-migration/5.undo.sql \
	workers/gearbox/sql/status-migration/6.undo.sql \
	workers/gearbox/sql/status-migration/7.sql \
	workers/gearbox/sql/status-migration/7.undo.sql \
	workers/gearbox/sql/status-migration/8.sql \
	workers/gearbox/sql/status-migration/8.undo.sql \
	workers/gearbox/sql/status-migration/9.sql \
	workers/gearbox/sql/status-migration/9.undo.sql \
	$(NULL)

workers/gearbox/conf/.handlers.d/%:
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/gearbox/conf/.handlers.d < workers/gearbox/WorkerGearbox.cc

gbhandlersdir = ${sysconfdir}/gearbox/gearbox-handlers.d/
dist_gbhandlers_DATA = workers/gearbox/conf/.handlers.d/*

# on install generate directories needed by gearbox daemons
install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/counters
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/test-agents
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/test-basic
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/test-cancel
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/test-chained
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/test-delay
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/test-sync
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/test-agents-perl
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/test-basic-perl
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/test-cancel-perl
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/test-chained-perl
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/test-delay-perl
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/test-sync-perl
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/gearbox/db/test-basic-python
	$(mkinstalldirs) $(DESTDIR)/var/log/gearbox

bin_PROGRAMS+=workers/test-basic/workerTestBasic
workers_test_basic_workerTestBasic_SOURCES = workers/test-basic/WorkerTestBasic.h workers/test-basic/WorkerTestBasic.cc workers/test-basic/workerTestBasicMain.cc
workers_test_basic_workerTestBasic_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB)
workers_test_basic_workerTestBasic_LDADD = gearbox/core/libgearbox_core.la gearbox/store/libgearbox_store.la gearbox/worker/libgearbox_worker.la gearbox/job/libgearbox_job.la

bin_PROGRAMS+=workers/test-agents/workerTestAgents
workers_test_agents_workerTestAgents_SOURCES = workers/test-agents/WorkerTestAgents.h workers/test-agents/WorkerTestAgents.cc workers/test-agents/workerTestAgentsMain.cc
workers_test_agents_workerTestAgents_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB)
workers_test_agents_workerTestAgents_LDADD = gearbox/core/libgearbox_core.la gearbox/store/libgearbox_store.la gearbox/worker/libgearbox_worker.la gearbox/job/libgearbox_job.la

bin_PROGRAMS+=workers/test-cancel/workerTestCancel
workers_test_cancel_workerTestCancel_SOURCES = workers/test-cancel/WorkerTestCancel.h workers/test-cancel/WorkerTestCancel.cc workers/test-cancel/workerTestCancelMain.cc
workers_test_cancel_workerTestCancel_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB)
workers_test_cancel_workerTestCancel_LDADD = gearbox/core/libgearbox_core.la gearbox/store/libgearbox_store.la gearbox/worker/libgearbox_worker.la gearbox/job/libgearbox_job.la

bin_PROGRAMS+=workers/test-chained/workerTestChained
workers_test_chained_workerTestChained_SOURCES = workers/test-chained/WorkerTestChained.h workers/test-chained/WorkerTestChained.cc workers/test-chained/workerTestChainedMain.cc
workers_test_chained_workerTestChained_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB)
workers_test_chained_workerTestChained_LDADD = gearbox/core/libgearbox_core.la gearbox/store/libgearbox_store.la gearbox/worker/libgearbox_worker.la gearbox/job/libgearbox_job.la

bin_PROGRAMS+=workers/test-delay/workerTestDelay
workers_test_delay_workerTestDelay_SOURCES = workers/test-delay/WorkerTestDelay.h workers/test-delay/WorkerTestDelay.cc workers/test-delay/workerTestDelayMain.cc
workers_test_delay_workerTestDelay_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB)
workers_test_delay_workerTestDelay_LDADD = gearbox/core/libgearbox_core.la gearbox/store/libgearbox_store.la gearbox/worker/libgearbox_worker.la gearbox/job/libgearbox_job.la

bin_PROGRAMS+=workers/test-sync/workerTestSync
workers_test_sync_workerTestSync_SOURCES = workers/test-sync/WorkerTestSync.h workers/test-sync/WorkerTestSync.cc workers/test-sync/workerTestSyncMain.cc
workers_test_sync_workerTestSync_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB)
workers_test_sync_workerTestSync_LDADD = gearbox/core/libgearbox_core.la gearbox/store/libgearbox_store.la gearbox/worker/libgearbox_worker.la gearbox/job/libgearbox_job.la

# configuration for test workers
dist_schemapkgdata_DATA += \
	workers/test-agents/schemas/get-testagents-thing-v1.js \
	workers/test-agents/schemas/delete-testagents-thing-v1.js \
	workers/test-agents/schemas/create-testagents-thing-v1.js \
	workers/test-agents/perl/schemas/get-testagentsperl-thing-v1.js \
	workers/test-agents/perl/schemas/delete-testagentsperl-thing-v1.js \
	workers/test-agents/perl/schemas/create-testagentsperl-thing-v1.js \
	workers/test-basic/schemas/update-testbasic-thing-v1.js \
	workers/test-basic/schemas/get-testbasic-thing-v1.js \
	workers/test-basic/schemas/delete-testbasic-thing-v1.js \
	workers/test-basic/schemas/create-testbasic-thing-v1.js \
    workers/test-basic/perl/schemas/update-testbasicperl-thing-v1.js \
    workers/test-basic/perl/schemas/get-testbasicperl-thing-v1.js \
    workers/test-basic/perl/schemas/delete-testbasicperl-thing-v1.js \
    workers/test-basic/perl/schemas/create-testbasicperl-thing-v1.js \
    workers/test-basic/python/schemas/update-testbasicpython-thing-v1.js \
    workers/test-basic/python/schemas/get-testbasicpython-thing-v1.js \
    workers/test-basic/python/schemas/delete-testbasicpython-thing-v1.js \
    workers/test-basic/python/schemas/create-testbasicpython-thing-v1.js \
	workers/test-cancel/schemas/update-testcancel-thing-v1.js \
	workers/test-cancel/schemas/create-testcancel-continuation-v1.js \
	workers/test-cancel/perl/schemas/update-testcancelperl-thing-v1.js \
	workers/test-cancel/perl/schemas/create-testcancelperl-continuation-v1.js \
	workers/test-chained/schemas/get-testchained-thing-v1.js \
	workers/test-chained/schemas/get-testchained-hello-v1.js \
	workers/test-chained/schemas/get-testchained-hello1-v1.js \
	workers/test-chained/schemas/get-testchained-goodbye-v1.js \
	workers/test-chained/schemas/delete-testchained-thing-v1.js \
	workers/test-chained/schemas/delete-testchained-service2-v1.js \
	workers/test-chained/schemas/create-testchained-thing-v1.js \
	workers/test-chained/schemas/create-testchained-service2-v1.js \
	workers/test-chained/schemas/create-testchained-hello2-v1.js \
	workers/test-chained/schemas/create-testchained-goodbye-v1.js \
	workers/test-chained/perl/schemas/get-testchainedperl-thing-v1.js \
	workers/test-chained/perl/schemas/get-testchainedperl-hello-v1.js \
	workers/test-chained/perl/schemas/get-testchainedperl-hello1-v1.js \
	workers/test-chained/perl/schemas/get-testchainedperl-goodbye-v1.js \
	workers/test-chained/perl/schemas/delete-testchainedperl-thing-v1.js \
	workers/test-chained/perl/schemas/delete-testchainedperl-service2-v1.js \
	workers/test-chained/perl/schemas/create-testchainedperl-thing-v1.js \
	workers/test-chained/perl/schemas/create-testchainedperl-service2-v1.js \
	workers/test-chained/perl/schemas/create-testchainedperl-hello2-v1.js \
	workers/test-chained/perl/schemas/create-testchainedperl-goodbye-v1.js \
	workers/test-delay/schemas/get-testdelay-counter-v1.js \
	workers/test-delay/schemas/delete-testdelay-counter-v1.js \
	workers/test-delay/schemas/create-testdelay-counter-v1.js \
	workers/test-delay/perl/schemas/get-testdelayperl-counter-v1.js \
	workers/test-delay/perl/schemas/delete-testdelayperl-counter-v1.js \
	workers/test-delay/perl/schemas/create-testdelayperl-counter-v1.js \
	workers/test-sync/schemas/update-testsync-thing-v1.js \
	workers/test-sync/schemas/get-testsync-thing-v1.js \
	workers/test-sync/schemas/delete-testsync-thing-v1.js \
	workers/test-sync/schemas/create-testsync-thing-v1.js \
	workers/test-sync/perl/schemas/update-testsyncperl-thing-v1.js \
	workers/test-sync/perl/schemas/get-testsyncperl-thing-v1.js \
	workers/test-sync/perl/schemas/delete-testsyncperl-thing-v1.js \
	workers/test-sync/perl/schemas/create-testsyncperl-thing-v1.js \
	$(NULL)


initdsysconfdir = ${sysconfdir}/init.d/
dist_initdsysconf_DATA = \
	workers/gearbox/conf/init.d/gearbox \
	workers/gearbox/conf/init.d/gearbox-test-workers \
	$(NULL)

dist_bin_SCRIPTS = \
	workers/test-agents/perl/workerTestAgents.pl \
	workers/test-basic/perl/workerTestBasic.pl \
	workers/test-cancel/perl/workerTestCancel.pl \
	workers/test-chained/perl/workerTestChained.pl \
	workers/test-delay/perl/workerTestDelay.pl \
	workers/test-sync/perl/workerTestSync.pl \
    workers/test-basic/python/workerTestBasic.py \
	$(NULL)

dist_gbsysconf_DATA += \
	workers/test-agents/conf/test-agents.conf \
	workers/test-agents/conf/test-agents-agents.conf \
	workers/test-agents/perl/conf/test-agents-perl.conf \
	workers/test-agents/perl/conf/test-agents-perl-agents.conf \
	workers/test-basic/conf/test-basic.conf \
    workers/test-basic/perl/conf/test-basic-perl.conf \
    workers/test-basic/python/conf/test-basic-py.conf \
	workers/test-cancel/conf/test-cancel.conf \
	workers/test-cancel/perl/conf/test-cancel-perl.conf \
	workers/test-chained/conf/test-chained.conf \
	workers/test-chained/perl/conf/test-chained-perl.conf \
	workers/test-delay/conf/test-delay.conf \
	workers/test-delay/perl/conf/test-delay-perl.conf \
	workers/test-sync/conf/test-sync.conf \
	workers/test-sync/perl/conf/test-sync-perl.conf \
    $(NULL)

workers/.test-handlers.d/%:
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d < workers/test-agents/WorkerTestAgents.cc && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-basic/WorkerTestBasic.cc && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-cancel/WorkerTestCancel.cc && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-chained/WorkerTestChained.cc && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-delay/WorkerTestDelay.cc && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-sync/WorkerTestSync.cc && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-agents/perl/workerTestAgents.pl && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-basic/perl/workerTestBasic.pl && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-cancel/perl/workerTestCancel.pl && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-chained/perl/workerTestChained.pl && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-delay/perl/workerTestDelay.pl && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-sync/perl/workerTestSync.pl && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-agents/php/workerTestAgents.php && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-basic/php/workerTestBasic.php && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-cancel/php/workerTestCancel.php && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-chained/php/workerTestChained.php && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-delay/php/workerTestDelay.php && \
	tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-sync/php/workerTestSync.php && \
    tools/gearbox_tools/gb-gen-handlers --handlers-dir workers/.test-handlers.d --update < workers/test-basic/python/workerTestBasic.py
    
testhandlersdir = ${sysconfdir}/gearbox/test-workers-handlers.d/
dist_testhandlers_DATA = workers/.test-handlers.d/*

###############################################
### gearbox/swig/perl
###############################################
perlarchlibdir = ${PERL_SITEARCHDIR}
perlarchlib_LTLIBRARIES=gearbox/swig/perl/libGearbox.la

perllibdir=${PERL_SITELIBDIR}
perllib_HEADERS = \
	gearbox/swig/perl/lib/Gearbox.pm

gbperllibdir = ${perllibdir}/Gearbox
gbperllib_HEADERS = \
	gearbox/swig/perl/lib/Gearbox/ConfigFile.pm \
	gearbox/swig/perl/lib/Gearbox/Errors.pm \
	gearbox/swig/perl/lib/Gearbox/JobManager.pm \
	gearbox/swig/perl/lib/Gearbox/Logger.pm \
	gearbox/swig/perl/lib/Gearbox/StatusManager.pm \
	gearbox/swig/perl/lib/Gearbox/Worker.pm

gearbox/swig/perl/lib/Gearbox/Errors.pm: gearbox/core/Errors.h
	gearbox/swig/perl/genErrors > gearbox/swig/perl/lib/Gearbox/Errors.pm

gearbox_swig_perl_libGearbox_la_CXXFLAGS = $(filter-out $(PERL_ARCH),$(PERL_CFLAGS))
gearbox_swig_perl_libGearbox_la_LDFLAGS = -avoid-version -shared $(filter-out $(PERL_ARCH),$(PERL_LDFLAGS))
gearbox_swig_perl_libGearbox_la_LIBADD = gearbox/core/libgearbox_core.la gearbox/job/libgearbox_job.la gearbox/worker/libgearbox_worker.la
gearbox_swig_perl_libGearbox_la_includes = \
	gearbox/swig/SwigWorker.h \
    $(NULL)

gearbox_swig_perl_libGearbox_la_SOURCES = $(gearbox_swig_perl_libGearbox_la_includes) \
	gearbox/swig/perl/SwigGearbox_wrap.cc \
	gearbox/swig/SwigWorker.cc \
    $(NULL)

gearbox/swig/perl/SwigGearbox_wrap.cc gearbox/swig/perl/SwigGearbox_wrap.h gearbox/swig/perl/lib/Gearbox.pm: gearbox/swig/SwigGearbox.i gearbox/swig/perl/perl.i gearbox/swig/perl/perlworker.i
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
	gearbox/swig/php/php_Json_helper.h \
	gearbox/swig/php/SwigGearbox_wrap.h \
	gearbox/swig/SwigWorker.h \
	$(NULL)

gearbox_swig_php_SwigGearbox_la_SOURCES = $(gearbox_swig_php_SwigGearbox_la_includes) \
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
else
# hack so "make dist" is happy when we disable php
gearbox/swig/php/php_SwigGearbox.h gearbox/swig/php/SwigGearbox.php gearbox/swig/php/SwigGearbox_wrap.cc gearbox/swig/php/SwigGearbox_wrap.h:
	touch $@
endif

###############################################
### gearbox/swig/python
###############################################
pythonarchlibdir=${PYTHON_SITEARCHDIR}
pythonarchlib_LTLIBRARIES = gearbox/swig/python/_gearbox.la

pythonlibdir = ${PYTHON_SITE_PKG}
pythonlib_HEADERS = gearbox/swig/python/lib/gearbox.py

gearbox_swig_python__gearbox_la_CPPFLAGS = $(SWIG_PYTHON_CPPFLAGS)
gearbox_swig_python__gearbox_la_LDFLAGS = -module -avoid-version -shared $(SWIG_PYTHON_LIBS)
gearbox_swig_python__gearbox_la_LIBADD = gearbox/core/libgearbox_core.la gearbox/job/libgearbox_job.la gearbox/worker/libgearbox_worker.la
gearbox_swig_python__gearbox_la_includes = \
    gearbox/swig/python/SwigGearbox_wrap.h \
	gearbox/swig/SwigWorker.h \
    $(NULL)

gearbox_swig_python__gearbox_la_SOURCES = $(gearbox_swig_python__gearbox_la_includes) \
    gearbox/swig/python/SwigGearbox_wrap.cc \
	gearbox/swig/SwigWorker.cc \
    $(NULL)

gearbox/swig/python/lib/gearbox.py gearbox/swig/python/SwigGearbox_wrap.cc gearbox/swig/python/SwigGearbox_wrap.h: gearbox/swig/SwigGearbox.i gearbox/swig/python/python.i gearbox/swig/python/pythonworker.i gearbox/swig/python/errors.i
	$(SWIG) -python -c++ -o gearbox/swig/python/SwigGearbox_wrap.cc -outdir gearbox/swig/python/lib -I$(abs_top_srcdir) gearbox/swig/SwigGearbox.i

gearbox/swig/python/errors.i: gearbox/core/Errors.h gearbox/swig/perl/genErrors
	echo '# this file is autogenerated by the toplevel Makefile.am' > gearbox/swig/python/errors.i
	echo '%pythoncode %{' >> gearbox/swig/python/errors.i
	gearbox/swig/perl/genErrors python >> gearbox/swig/python/errors.i
	echo '%}' >> gearbox/swig/python/errors.i


CLEANFILES += \
	gearbox/swig/python/errors.i \
    gearbox/swig/python/lib/gearbox.py \
    gearbox/swig/python/lib/gearbox.pyc \
    gearbox/swig/python/SwigGearbox_wrap.cc \
    gearbox/swig/python/SwigGearbox_wrap.h \
	gearbox/t/swig/python/setup.pyc \
    $(NULL)

###############################################
### apache module mod_gearbox
###############################################
apachemoddir = ${APACHE_LIBEXECDIR}
apachemod_LTLIBRARIES=apache/mod_gearbox/mod_gearbox.la

apache_mod_gearbox_mod_gearbox_la_CXXFLAGS = ${APACHE_CFLAGS}
apache_mod_gearbox_mod_gearbox_la_LDFLAGS = ${APR_LDFLAGS} -module -version-info $(MOD_GEARBOX_LIBRARY_VERSION)
apache_mod_gearbox_mod_gearbox_la_LIBADD = gearbox/core/libgearbox_core.la gearbox/job/libgearbox_job.la
apache_mod_gearbox_mod_gearbox_la_SOURCES = apache/mod_gearbox/mod_gearbox.cc

apacheconfddir = ${APACHE_CONFD_DIR}
dist_apacheconfd_DATA = apache/mod_gearbox/gearbox.conf

dist_gbconflogger_DATA += apache/mod_gearbox/httpd-logger.conf

###############################################
### apache modules for test workers
###############################################
dist_apacheconfd_DATA += \
	workers/test-agents/conf/httpd-test-agents.conf \
	workers/test-agents/perl/conf/httpd-test-agents-perl.conf \
	workers/test-basic/conf/httpd-test-basic.conf \
    workers/test-basic/perl/conf/httpd-test-basic-perl.conf \
    workers/test-basic/python/conf/httpd-test-basic-py.conf \
	workers/test-cancel/conf/httpd-test-cancel.conf \
	workers/test-cancel/perl/conf/httpd-test-cancel-perl.conf \
	workers/test-chained/conf/httpd-test-chained.conf \
	workers/test-chained/perl/conf/httpd-test-chained-perl.conf \
	workers/test-delay/conf/httpd-test-delay.conf \
	workers/test-delay/perl/conf/httpd-test-delay-perl.conf \
	workers/test-sync/conf/httpd-test-sync.conf \
	workers/test-sync/perl/conf/httpd-test-sync-perl.conf \
	$(NULL)

###############################################
### plugins/status/sql
###############################################
gblibexecdir = ${libexecdir}/gearbox
gbstatusplugindir = ${gblibexecdir}/plugins/status
gbstatusplugin_LTLIBRARIES=plugins/status/sql/sql_status.la

plugins_status_sql_sql_status_la_LDFLAGS = $(BOOST_LDFLAGS) $(SOCI_CORE_LIB) -module -version-info $(GEARBOX_SQL_STATUS_PLUGIN_LIBRARY_VERSION)
plugins_status_sql_sql_status_la_LIBADD = gearbox/core/libgearbox_core.la gearbox/store/libgearbox_store.la gearbox/job/libgearbox_job.la

plugins_status_sql_sql_status_la_SOURCES = \
	plugins/status/sql/SQLStatusImpl.cc \
	plugins/status/sql/SQLStatusImpl.h

###############################################
### tools
###############################################
dist_bin_SCRIPTS += \
	tools/gearbox_tools/gbtrace \
	tools/gearbox_tools/gbq \
	tools/gearbox_tools/gb-gen-handlers \
	tools/svcbuilder/bin/gearbox-svc-builder

tmplpkgdatadir = ${pkgdatadir}/gbtrace/templates
dist_tmplpkgdata_DATA = tools/gearbox_tools/templates/trace.tt

gbperllib_HEADERS += \
	tools/svcbuilder/lib/Gearbox/Utils.pm

svcperllibdir = ${gbperllibdir}/Service
svcperllib_HEADERS = \
	tools/svcbuilder/lib/Gearbox/Service/Builder.pm \
	tools/svcbuilder/lib/Gearbox/Service/Entry.pm

dist_gblibexec_SCRIPTS = common/sql/dbsetup

EXTRA_DIST = \
	LICENSE \
	LICENSE.Artistic \
	autogen.sh \
	auto/testAutoMake.sh \
	gearbox/swig/perl/perl.i \
	gearbox/swig/perl/perlworker.i \
	gearbox/swig/php/php.i \
	gearbox/swig/php/phpworker.i \
	gearbox/swig/python/errors.i \
	gearbox/swig/python/python.i \
	gearbox/swig/python/pythonworker.i \
	gearbox/swig/SwigGearbox.i \
	gearbox/swig/perl/genErrors \
	common

include rpm.am
