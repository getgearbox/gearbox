check_PROGRAMS = \
	worker.t \
	status.t \
	valid-schemas.t

worker_t_SOURCES = worker.cc
status_t_SOURCES = status.cc
valid_schemas_t_SOURCES = valid-schemas.cc

CXXFLAGS += -Wno-sign-compare $(TAPXX_CFLAGS) $(SOCI_SQLITE3_CFLAGS)
LDADD = ../libgearbox_worker.la
LDFLAGS  +=  $(TAPXX_LIBS) $(SOCI_SQLITE3_LIBS)

# find * \! -name Makefile\* \! -name \*.cc \! -path \*/.* \! -type d | xargs echo
EXTRA_DIST =  mkdb unit.conf

.PHONY : run test tap

run : $(check_PROGRAMS)
	for t in *.t; do \
		./$$t; \
	done;

tap:: TAP=1
tap:: TDIR=test_results
tap:: check
test: check
check:
	if [ -n "$(TAP)" ]; then \
		[ -d ../../../$(TDIR) ] || mkdir ../../../$(TDIR);  \
			dir=$$(cd .. && basename $$(pwd)); \
			for t in *.t; do \
				./$$t > ../../../$(TDIR)/$$dir-$$t.tap 2>&1; \
			done; \
	else \
		[ -n "$$NO_TESTS" ] || PATH=$$PATH:. /home/y/bin/prove -e ""   $(check_PROGRAMS); \
	fi

CLEANFILES = *.gcov *.gcda *.gcno *.info *.info.fixup

../%:
	$(MAKE) -C $(@D) $(@F)
