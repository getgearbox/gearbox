TOP=$(shell cd ../../.. && pwd)
include $(TOP)/common/make/Make.defs

TESTS	= $(patsubst %.cc,%,$(wildcard *.cc))

LINC += $(TAP_CFLAGS) $(SOCI_SQLITE3_CFLAGS)
LLDLIBS	= $(TAP_LIBS) -lWorkerGearbox $(SOCI_SQLITE3_LIBS) -lboost_filesystem $(LOG4CXX_LIBS) -lgearbox_worker

include $(TOP)/common/make/Make.rules
