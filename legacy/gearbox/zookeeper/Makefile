TOP=$(shell cd ../.. && pwd)

include $(TOP)/common/make/Make.defs

CXXSRC	= ZooKeeper.cc 
LIB_NAME = gearbox_zookeeper
SHLIB_VERSION = 0
EXETARGET = gearbox_zkutil
LDEF += 
LINC +=
LLDLIBS	+= -lzookeeper_st -lgearbox_core 

include $(TOP)/common/make/Make.rules

# Link dependencies specific to gearbox_zkutil
$(patsubst %,$(OBJDIR)/%,gearbox_zkutil): LLDLIBS += -lboost_program_options -l$(LIB_NAME) -L$(OBJDIR)
