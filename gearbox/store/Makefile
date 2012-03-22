TOP=$(shell cd ../.. && pwd)

include $(TOP)/common/make/Make.defs

CXXSRC	= $(wildcard *.cc)
LIB_NAME = gearbox_store
SHLIB_VERSION = 0
EXETARGET = 
LDEF += 
LINC += $(SOCI_CFLAGS)
LLDLIBS	+= $(SOCI_LIBS) -lgearbox_core

include $(TOP)/common/make/Make.rules
