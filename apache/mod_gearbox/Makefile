TOP=$(shell cd ../.. && pwd)
include $(TOP)/common/make/Make.defs

CXXSRC	= $(wildcard *.cc)
LIB_NAME = mod_gearbox
SHLIB_VERSION = handler
CXXFLAGS += $(YAPACHE_CFLAGS)
LINC +=
LLDLIBS	+= $(YAPACHE_LDLIBS) -lgearbox_core -lgearbox_job

include $(TOP)/common/make/Make.rules
