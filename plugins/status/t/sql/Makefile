TOP=$(shell cd ../../../.. && pwd)
include $(TOP)/common/make/Make.defs

TESTS	= $(patsubst %.cc,%,$(wildcard *.cc))

UNITPREFIX=
ifeq ($(PLATFORM_CURRENT),i386-rhel4-gcc3)
UNITPREFIX := 32-$(UNITPREFIX)
else
ifeq ($(PLATFORM_CURRENT),x86_64-rhel4-gcc3)
UNITPREFIX := 64-$(UNITPREFIX)
else
endif
endif
ifeq ($(MODE),gcov)
UNITPREFIX := gcov-$(UNITPREFIX)
endif

LDEF += -DUNITPREFIX='"$(UNITPREFIX)"'
LINC += $(TAP_CFLAGS) $(SOCI_SQLITE3_CFLAGS)
LLDLIBS	= $(TAP_LIBS) -lgearbox_store -lgearbox_job $(SOCI_SQLITE3_LIBS)

include $(TOP)/common/make/Make.rules
