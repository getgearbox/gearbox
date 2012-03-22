DIST_SUBDIRS = t
if WITH_TESTS
SUBDIRS = . t
endif

lib_LTLIBRARIES =  libgearbox_worker.la

libgearbox_worker_la_SOURCES = \
	Worker.cc

libgearbox_worker_la_CFLAGS=
libgearbox_worker_la_LIBADD= ../core/libgearbox_core.la ../gearman/libgearbox_gearman.la ../store/libgearbox_store.la
libgearbox_worker_la_DEPENDENCIES = $(libgearbox_worker_la_LIBADD)

gearboxdatadir=${datadir}/gearbox
nobase_dist_gearboxdata_DATA = \
	schemas/job-envelope.js \
	schemas/create-global-status-v1.js \
	schemas/error-response.js \
	schemas/get-global-status-response-v1.js

noinst_PROGRAMS = linktest

linktest_LDFLAGS = -Wl,-no-undefined -Wl,-x
linktest_LDADD = libgearbox_worker.la
nodist_linktest_SOURCES = linktest.cc
linktest.cc : libgearbox_worker.la
	echo "int main (int argc __attribute__((unused)), char** argv __attribute__((unused))) { return 0; }" > linktest.cc

CLEANFILES = *.t *.gcov *.gcda *.gcno *.info *.info.fixup linktest.cc

if WITH_TESTS

check_PROGRAMS = test async_client

test_SOURCES = test.cc
test_CXXFLAGS = $(SOCI_MYSQL_CFLAGS)
test_LDFLAGS = $(SOCI_MYSQL_LIBS)
test_LDADD = ./libgearbox_worker.la

async_client_SOURCES = async_client.cc
async_client_LDADD = ./libgearbox_worker.la

endif

../core/% ../gearman/% ../store/%:
	$(MAKE) -C $(@D) $(@F)

