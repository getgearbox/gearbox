######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################
package Gearbox::Logger;
use strict;
use warnings;

use Gearbox;
use Log::Log4perl qw(:easy);
use Exporter;
use base qw(Exporter);
our @EXPORT = qw( _TRACE _DEBUG _INFO _WARN _ERROR _FATAL );

# turn on auto flush 
$|++;

sub init {
    my ($class, $config) = @_;
    Gearbox::log_init( $config );
    my $cfg = Gearbox::ConfigFile->new($config);
    Log::Log4perl::init( $cfg->get_string_default("log", "config_file", "/home/y/conf/gearbox/logger/gearbox-logger.conf" ) );
    Log::Log4perl::MDC->put("pid" => $$);
}

*_TRACE  = \&TRACE;
*_DEBUG  = \&DEBUG;
*_INFO   = \&INFO;
*_WARN   = \&WARN;
*_ERROR  = \&ERROR;
*_FATAL  = \&FATAL;

1;



