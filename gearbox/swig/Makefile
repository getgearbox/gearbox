TOP=../..
include $(TOP)/common/make/Make.defs

SUBDIRS = php

# can't build 64 bit perl swig bindings because no 64 bit -lperl exists
# so if arch is 32bit or we are building for 32bit then recurse into perl subdir
ifneq ($(filter i386%,$(shell uname -i) $(PLATFORM_CURRENT)),)
SUBDIRS += perl
endif

# recurse into perl if building packages
pkg%:: SUBDIRS += perl

include $(TOP)/common/make/Make.rules
