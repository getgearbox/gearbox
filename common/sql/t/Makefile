TOP=$(shell cd ../../.. && pwd)
include $(TOP)/common/make/Make.defs

override PLATFORMS := 
RUNTESTS	= $(wildcard *.t)

include $(TOP)/common/make/Make.rules

clean::
	./rmdb
