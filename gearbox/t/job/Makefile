TOP=$(shell cd ../../.. && pwd)
include $(TOP)/common/make/Make.defs

TESTS	= $(patsubst %.cc,%,$(wildcard *.cc))

LINC += $(TAP_CFLAGS) -I..
LLDLIBS	= $(TAP_LIBS) -lgearbox_job

include $(TOP)/common/make/Make.rules

GBDEPS  = core scoreboard job
libs:: $(GBDEPS)
$(GBDEPS):
	${MAKE} -C ../../$@ PLATFORM_CURRENT=$(filter-out .,$(basename $(OBJDIR))) MODE=$(patsubst .%,%,$(suffix $(OBJDIR))) default
.PHONY: $(GBDEPS)