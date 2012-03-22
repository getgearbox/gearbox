TOP=$(shell cd ../.. && pwd)

SUBDIRS=php perl

include /usr/share/gearbox_cfg/make/Make.defs

export YAUTOPROF_ENTRY=Worker.*::do_.*

CXXSRC	= $(filter-out $(EXETARGET).cc,$(wildcard *.cc))
LIB_NAME = WorkerTestBasic
EXETARGET = workerTestBasic
LDEF += 
LIBARS += $(OBJDIR)/lib$(LIB_NAME).a
LINC += $(BPO_CFLAGS)
LLDLIBS	+= -lgearbox_core -lgearbox_job -lgearbox_worker -lgearbox_store $(BPO_LIBS) 

include /usr/share/gearbox_cfg/make/Make.rules
