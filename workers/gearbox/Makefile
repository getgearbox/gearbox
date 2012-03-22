TOP=$(shell cd ../.. && pwd)

include /usr/share/gearbox_cfg/make/Make.defs

export YAUTOPROF_ENTRY=Worker.*::do_.*,Delay.*::run_.*,Delay.*::wait_.*

CXXSRC	= $(filter-out $(EXETARGET).cc,$(wildcard *.cc))
LIB_NAME = WorkerGearbox
EXETARGET = workerGearbox delayDaemon
LDEF += 
LINC += $(BPO_CFLAGS)
LIBARS += $(OBJDIR)/lib$(LIB_NAME).a
LLDLIBS	+= -lgearbox_core -lgearbox_worker -lgearbox_job -lgearbox_store $(BPO_LIBS) $(SOCI_SQLITE3_LIBS)

include /usr/share/gearbox_cfg/make/Make.rules
