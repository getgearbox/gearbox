TOP=$(shell cd ../.. && pwd)

include /usr/share/gearbox_cfg/make/Make.defs

SUBDIRS = php perl

export YAUTOPROF_ENTRY=Worker.*::do_.*

CXXSRC	= $(filter-out $(EXETARGET).cc,$(wildcard *.cc))
LIB_NAME = WorkerTestChained
EXETARGET = workerTestChained
LDEF += 
LIBARS += $(OBJDIR)/lib$(LIB_NAME).a
LINC += $(BPO_CFLAGS)
LLDLIBS	+= -lgearbox_core -lgearbox_job -lgearbox_worker -lgearbox_store $(BPO_LIBS)

include /usr/share/gearbox_cfg/make/Make.rules
