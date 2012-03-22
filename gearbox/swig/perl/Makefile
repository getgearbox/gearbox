TOP=$(shell cd ../../.. && pwd)

include /home/y/share/gearbox_cfg/make/Make.defs

ifneq ($(PERLXX),)
WARN=
CXXSRC	= $(wildcard *.cc) SwigGearbox_wrap.cc 
LIB_NAME = Gearbox
SHLIB_VERSION = handler
EXETARGET = 
LDEF += 
LINC += $(shell $(PERLXX) -MExtUtils::Embed -e ccopts) -I../
LLDLIBS	+= -lgearbox_core -lgearbox_job -lgearbox_worker $(shell $(PERLXX) -MExtUtils::Embed -e ldopts)
endif

PLATFORMS := i386-rhel4-gcc3

include /home/y/share/gearbox_cfg/make/Make.rules

ifeq ($(PERLXX),)
all:: perl58
ifneq ($(OBJDIR),.)
ifeq ($(MODE),)
all:: perl510
endif
endif
endif

perl58:
	$(MAKE) $(MAKEFILEARG) PERLXX=/home/y/bin/perl5.8
	if [ "$(OBJDIR)" != "." ]; then \
		mkdir -p $(OBJDIR)-perl58; \
		mv $(OBJDIR)/* $(OBJDIR)-perl58; \
	fi

perl510:
	$(MAKE) $(MAKEFILEARG) PERLXX=/home/y/bin/perl5.10
	mkdir -p $(OBJDIR)-perl510
	mv $(OBJDIR)/* $(OBJDIR)-perl510

SwigGearbox_wrap.cc SwigGearbox_wrap.h: ../SwigGearbox.i perl.i perlworker.i
	/home/y/bin64/swig -perl -c++ -o SwigGearbox_wrap.cc -I$(TOP) ../SwigGearbox.i 
	./genErrors > lib/Gearbox/Errors.pm

clean::
	rm -rf *.a *.depend *.o *_wrap.* Gearbox.pm CHANGELOG

default:
	$(MAKE) quick
