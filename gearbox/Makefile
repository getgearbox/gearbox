include ../common/make/Make.defs

SUBDIRS = core scoreboard store job worker
# dont run swig under coverity, it will produce a ton of bogus alerts
ifneq ($(MODE),scov)
SUBDIRS += swig
endif

ifeq ($(NO_TESTS),)
SUBDIRS += t
endif

include ../common/make/Make.rules
