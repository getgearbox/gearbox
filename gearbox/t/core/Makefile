TOP=$(shell cd ../../.. && pwd)
include $(TOP)/common/make/Make.defs

TESTS	= $(patsubst %.cc,%,$(wildcard *.cc))

# for loop kernel stuff
LINC += -I/usr/src/kernels/`uname -r`-`uname -m`/include

LDEF += -DTESTPLUGINDIR='"plugins/$(OBJDIR)"'
LINC += $(TAP_CFLAGS)
LLDLIBS	= $(TAP_LIBS) -lgearbox_core

SUBDIRS = plugins

include $(TOP)/common/make/Make.rules

GBDEPS  = core
libs:: $(GBDEPS)
$(GBDEPS):
	${MAKE} -C ../../$@ PLATFORM_CURRENT=$(filter-out .,$(basename $(OBJDIR))) MODE=$(patsubst .%,%,$(suffix $(OBJDIR))) default
.PHONY: $(GBDEPS)