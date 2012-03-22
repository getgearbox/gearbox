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
# generate handlers links
#
# $ gen_handlers_dir.pl < worker.cc
#
# then look in .handlers.d for symlinks to dev null

use warnings;
use strict;
my $handlers;
my $code = do {
    local $/;
    <>
};
while( $code =~ /(?:WORKER_REGISTER\(\s*\S+\s*,\s*|->register_handler\(\s*\")([^")\s]+)/sg ) {
     my $handler = $1;
     $handler =~ /^do_\w+_(\w+)_\w+_\w+/;
     push @{$handlers->{$1}}, $handler;
}

system("rm -rf ./.handlers.d");
mkdir("./.handlers.d");

foreach my $key (keys %$handlers) {
    foreach my $handler (sort @{$handlers->{$key}}) {
        system("ln -s /dev/null ./.handlers.d/$handler");
    }
}
