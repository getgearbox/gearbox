TOP=$(shell cd ../../../.. && pwd)

include $(TOP)/common/make/Make.defs

ifndef LIB_NAME

all clean ::
	$(MAKE) $(MAKEFILEARG) SHLIB_VERSION=handler LIB_NAME=hello CXXSRC="TestPlugin.cc hello.cc" $@
	$(MAKE) $(MAKEFILEARG) SHLIB_VERSION=handler LIB_NAME=goodbye CXXSRC="TestPlugin.cc goodbye.cc" $@

endif

CXXSRC	= 
LIB_NAME =
SHLIB_VERSION =
EXETARGET = 
# for look kernel stuff
LINC += 
LLDLIBS	+= -lgearbox_core

include $(TOP)/common/make/Make.rules
