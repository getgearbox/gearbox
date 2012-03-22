TOP=../../..
include ../../../common/make/Make.defs

SUBDIRS = 

# can't build 64 bit perl swig bindings because no 64 bit -lperl exists
# so if arch is 32bit or we are building for 32bit then recurse into perl subdir
ifneq ($(filter i386%,$(shell uname -i) $(PLATFORM_CURRENT)),)
ifeq ($(MODE),)
SUBDIRS += perl
endif
endif

include ../../../common/make/Make.rules
