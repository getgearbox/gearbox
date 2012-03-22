TOP=$(shell cd ../.. && pwd)

include $(TOP)/common/make/Make.defs

CXXSRC	= $(wildcard *.cc)
LIB_NAME = gearbox_job
SHLIB_VERSION = 0
EXETARGET = 
LDEF += 
LINC += $(GEARMAN_CLFAGS)
LLDLIBS	+= $(GEARMAN_LIBS) -lgearbox_core -lboost_graph 

include $(TOP)/common/make/Make.rules
