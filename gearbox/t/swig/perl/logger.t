#!/usr/bin/env perl
use lib $ENV{PWD}."/".($0 =~ m{(.*)/[^/]+$})[0];
use strict;
use warnings;
use setup;
use Test::Trivial (tests => 7);

use Gearbox::Logger;
NOTHROW { Gearbox::Logger->init("./unit.conf") };

# trace not enabled, returns false
NOK _TRACE("trace");

# all others enabled, returns true
OK _DEBUG("debug");
OK _INFO("info");
OK _WARN("warn");
OK _ERROR("error");
OK _FATAL("fatal");

