TOP=$(shell cd ../../.. && pwd)
include $(TOP)/common/make/Make.defs

TESTS	= $(patsubst %.cc,%,$(wildcard *.cc))

LINC += $(TAP_CFLAGS) $(SOCI_SQLITE3_CFLAGS)
LLDLIBS	= $(TAP_LIBS) -lgearbox_worker $(SOCI_SQLITE3_LIBS)

include $(TOP)/common/make/Make.rules

GBDEPS  = core scoreboard job store worker
libs:: $(GBDEPS)
$(GBDEPS):
	${MAKE} -C ../../$@ PLATFORM_CURRENT=$(filter-out .,$(basename $(OBJDIR))) MODE=$(patsubst .%,%,$(suffix $(OBJDIR))) default

.PHONY: $(GBDEPS)
