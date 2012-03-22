TOP=$(shell cd ../../../.. && pwd)
include $(TOP)/common/make/Make.defs

override PLATFORMS := i386-rhel4-gcc3

export OBJDIR
RUNTESTS	= $(wildcard *.t)

CXXSRC	= gearman_stub.cc
LIB_NAME = gearman_stub
SHLIB_VERSION = 0

LINC += $(TAP_CFLAGS)
LLDLIBS += -lgearbox_core

include $(TOP)/common/make/Make.rules
