TOP=$(shell cd ../.. && pwd)

include $(TOP)/common/make/Make.defs

EXETARGET = gearbox_json
EXETARGET_CXXSRC = $(patsubst %,%.cc,$(EXETARGET))
CXXSRC	= $(filter-out $(EXETARGET_CXXSRC),$(wildcard *.cc))
LIB_NAME = gearbox_core
SHLIB_VERSION = 0
# for look kernel stuff
LINC += -I/usr/src/kernels/`uname -r`-`uname -m`/include 
LINC += $(YAJL_CFLAGS) $(CURL_CFLAGS) $(BFS_CFLAGS) $(BIOS_CFLAGS) $(LOG4CXX_CFLAGS) $(BRX_CFLAGS) $(BIOS_CFLAGS)
LLDLIBS	= $(YAJL_LIBS) $(CURL_LIBS) $(BFS_LIBS) $(LOG4CXX_LIBS) $(BRX_LIBS) -lcrypto -luuid $(BIOS_LIBS)

include $(TOP)/common/make/Make.rules

# Link dependencies specific to gearbox_json
$(patsubst %,$(OBJDIR)/%,gearbox_json): LLDLIBS += -lboost_program_options -l$(LIB_NAME) -L$(OBJDIR)
