TOP=$(shell cd ../../.. && pwd)

include $(TOP)/common/make/Make.defs

CXXSRC	= $(wildcard *.cc)
LIB_NAME = sql_status
SHLIB_VERSION = handler
LINC += 
LLDLIBS	+= -lgearbox_job -lgearbox_store

include $(TOP)/common/make/Make.rules
