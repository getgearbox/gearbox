TOP=$(shell cd ../../.. && pwd)

include $(TOP)/common/make/Make.defs

ifneq ($(PHP_VERSION),)
WARN=
CXXSRC	= $(wildcard *.cc) SwigGearbox_wrap.cc 
LIB_NAME = SwigGearbox
SHLIB_VERSION = handler
EXETARGET = 
LDEF += 
LIBARS += $(OBJDIR)/lib$(LIB_NAME).a
LINC += -I/home/y/include/php-$(PHP_VERSION) -I/home/y/include/php-$(PHP_VERSION)/main -I/home/y/include/php-$(PHP_VERSION)/TSRM -I/home/y/include/php-$(PHP_VERSION)/Zend -I/home/y/include/php-$(PHP_VERSION)/ext -I/home/y/include/php-$(PHP_VERSION)/ext/date/lib -I..
LLDLIBS	+= -lgearbox_core -lgearbox_job -lgearbox_worker $(PHP_LIBS)
endif

include $(TOP)/common/make/Make.rules

ifeq ($(PHP_VERSION),)
all:: php53
ifneq ($(OBJDIR),.)
all:: php52 php51
endif
endif

php53:
	$(MAKE) $(MAKEFILEARG) PHP_VERSION=5.3 PHP_LIBS=-lphpstubs22-5.3
	if [ "$(OBJDIR)" != "." ]; then \
		mkdir -p $(OBJDIR)-php53; \
		mv $(OBJDIR)/* $(OBJDIR)-php53; \
	fi

ifneq ($(filter i386%,$(shell uname -i) $(PLATFORM_CURRENT)),)
php52:
	$(MAKE) $(MAKEFILEARG) PHP_VERSION=5.2 PHP_LIBS=-lphpstubs-5.2
	mkdir -p $(OBJDIR)-php52
	mv $(OBJDIR)/* $(OBJDIR)-php52

php51:
	$(MAKE) $(MAKEFILEARG) PHP_VERSION=5.1 PHP_LIBS=-lphpstubs-5.1
	mkdir -p $(OBJDIR)-php51
	mv $(OBJDIR)/* $(OBJDIR)-php51
else
php51 php52:
	echo "not building gearbox_php_lib on this platform"
endif

php_SwigGearbox.h SwigGearbox.php SwigGearbox_wrap.cc SwigGearbox_wrap.h: ../SwigGearbox.i php.i
	/home/y/bin64/swig -php -cppext cxx -c++ -o SwigGearbox_wrap.cc -I$(TOP) ../SwigGearbox.i
	perl -pi -e 's/(SWIG_SetPointerZval\(.*?),\s+0\)/$$1, 2)/g' SwigGearbox_wrap.cc
	perl -pi -e 's/Gearbox__//g' SwigGearbox_wrap.cc SwigGearbox.php
	perl -pi -e 's/new RealJobManager/new JobManager/g' SwigGearbox.php
	perl -pi -e 's/new RealScoreboard/new Scoreboard/g' SwigGearbox.php
	echo "<?php" > GearboxErrors.php
	cat ../../core/Errors.h | perl -ne 's/DEFINE_ERROR\((\w+),(\d+)\);/class ERR_$$1 extends Exception {\n  public function __construct(\$$message) {\n    parent::__construct("ERR_$$1: ".\$$message,$$2);\n  }\n}\n\nclass ERR_CODE_$$2 extends ERR_$$1 {}\n/ && print' >> GearboxErrors.php
	echo "?>" >> GearboxErrors.php

clean::
	rm -rf *.a php_SwigGearbox.h *.depend *.o SwigGearbox.php *_wrap.* GearboxErrors.php
