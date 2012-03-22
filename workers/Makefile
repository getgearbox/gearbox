include ../common/make/Make.defs

SUBDIRS =   gearbox \
			test-basic \
			test-cancel \
			test-sync \
			test-chained \
			test-agents \
			test-delay \
			$(NULL)

ifeq ($(NO_TESTS),)
SUBDIRS += t
endif

include ../common/make/Make.rules
