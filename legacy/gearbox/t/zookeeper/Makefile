TOP=$(shell cd ../../.. && pwd)
include $(TOP)/common/make/Make.defs

TESTS	= $(patsubst %.cc,%,$(wildcard *.cc))

LINC += $(TAP_CFLAGS)
LLDLIBS	= -lzookeeper_st $(TAP_LIBS) -lgearbox_zookeeper

include $(TOP)/common/make/Make.rules
