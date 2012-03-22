#!/bin/env perl
######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################
#
# generate table of handlers for the rest interface
#
# $ gen_handler_table.pl < workerCC.cc > cc_handlers.conf
#

use warnings;
use strict;

$/ = undef;
my $handlers = { 'global' => ['do_get_global_status_v1',
                              'do_create_global_status_v1',
                              'do_update_global_status_v1'] };
foreach my $x (split(/;/, <>)) {
  if ($x =~ /(?:WORKER_REGISTER\(\s*\S+\s*,\s*|this->register_handler\(\s*\")([^")\s]+)/s ) {
     my $handler = $1;
     $handler =~ /^do_\w+_(\w+)_\w+_\w+/;
     push @{$handlers->{$1}}, $handler;
  }
}

print <<EOM;
{
    // this list is automatically generated during the build.
    "handlers" : [
EOM

my $first = 1;
foreach my $key (keys %$handlers) {
    foreach my $handler (sort @{$handlers->{$key}}) {
        print !$first ? ",\n" : "";
        print "       \"$handler\"";
        $first = 0;
    }
}
print "\n";
print "    ]\n";
print "}\n";
