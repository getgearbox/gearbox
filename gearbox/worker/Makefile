TOP=$(shell cd ../.. && pwd)

include $(TOP)/common/make/Make.defs

CXXSRC	= $(filter-out test.cc async_client.cc,$(wildcard *.cc))
LIB_NAME = gearbox_worker
SHLIB_VERSION = 0
EXETARGET = 
LDEF += 
LINC += $(SOCI_CFLAGS)
LLDLIBS	+= $(SOCI_LIBS) -lgearbox_core -lgearbox_store -lgearbox_job -lgearman 

include $(TOP)/common/make/Make.rules

all:: examples

examples :: example_test example_async_test
example_test:: test.cc
example_async_test:: async_client.cc



