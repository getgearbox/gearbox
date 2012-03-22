#!/bin/env perl
######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################

use strict;
use warnings;

use Getopt::Long;
use JSON qw(to_json from_json);

my %opt;
GetOptions( \%opt,
    # go in worker mode (default is client mode)
    "worker!",

    # for both worker and client mode
    "function=s",
    "config=s",

    # for client output
    "headers!",
    "status!",

    # for client input
    "header|H=s%",
    "argument=s@",
    "matrix_argument=s%",
    "environ=s%",
    "query_param=s%",
);

if ( !$opt{function} ) {
    die usage("missing --function");
}

my $input = shift || do {
    local $/ = undef;
    <STDIN>;
};

my $conf = $opt{config} || "/etc/gearbox/client.conf";

# worker mode
if ( $opt{worker} ) { 
    my $worker = Gearbox::InlineWorker->new( $conf, $input );
    $worker->register_handler( $opt{function}, \&Gearbox::InlineWorker::do_work );
    $worker->run(); # never returns
}

# client mode
Gearbox::log_init( $conf );
my $jm = Gearbox::JobManager->new( Gearbox::ConfigFile->new( $conf ) );

# these are required, but not used :(
$jm->base_uri("http://localhost");
$jm->parent_uri("http://localhost");

my $job = $jm->job( $opt{function} );
$job->content( to_json( {
    input => $input
} ) );

if ( $opt{header} ) { 
    $job->headers( $opt{header} );
}
if ( $opt{matrix_argument} ) { 
    $job->matrix_arguments( $opt{matrix_argument} );
}
if ( $opt{argument} ) { 
    map { $job->add_argument( $_ ) } @{ $opt{argument} };
}
if ( $opt{environ} ) { 
    $job->environ( $opt{environ} );
}
if ( $opt{query_param} ) { 
    $job->query_params( $opt{query_param} );
}

$job->type( $Gearbox::Job::JOB_SYNC );
my $resp = $job->run();

if ( $opt{status} ) {
    # pretty print status
    print  to_json( from_json( $resp->status()->serialize() ), { pretty => 1 } );
}

if ( $opt{headers} ) {
    my $headers = $job->headers();
    foreach my $key ( keys %$headers ) {
        print "$key: $headers->{$key}\n";
    }
}

print to_json( from_json( $resp->content() ), { pretty => 1 } );

sub usage {
    my $msg = shift;
    $msg .= "\n" if $msg;

    return <<"EOM";
${msg}Usage: $0 -f <function name> [options] 

Global Options:
    -f/--function   must follow naming scheme do_<op>_<comp>_<rsrc>_<version>
    -c/--config     defaults to /etc/gearbox/client.conf

Worker Options (for creating worker):
    -w/--worker [command]   if command is null, read from stdin

Client Options (for calling worker):
    [content]        if content is null, read from stdin 

    -h/--headers     print response headers
    -s/--status      print response status

    -a/--argument    <arg>         can be used multiple times
    -H/--header      <key>=<value> can be used multiple times
    -m/--matrix_arg  <key>=<value> can be used multiple times
    -e/--environ     <key>=<value> can be used multiple times
    -e/--query_param <key>=<value> can be used multiple times
EOM
}


package Gearbox::InlineWorker;

use strict;
use warnings;

use Gearbox::Worker;
use base 'Gearbox::Worker';
use JSON qw(from_json to_json);
use IPC::Open3;
use Symbol 'gensym'; 

sub new {
    my $self = shift->SUPER::new(@_);
    $self->{cmd} = pop @_;
    return $self;
}

sub do_work { 
    my $self = shift;
    my $job = shift;
    my $resp = shift;

    my $cmd = $self->{cmd};
    my ( $wtr, $rdr, $errfh );
    $errfh = gensym;
    my $pid = open3( $wtr, $rdr, $errfh, $cmd );

    print $wtr $job->json_content()->{input};
    close $wtr;
    my $out = do {
        local $/ = undef;
        <$rdr>;
    };
    my $err = do {
        local $/ = undef;
        <$errfh>;
    };
    
    $resp->content( to_json( {
        command => $cmd,
        stdout => $out,
        stderr => $err,
    } ) );

    waitpid( $pid, 0 );

    return $WORKER_SUCCESS;
}
