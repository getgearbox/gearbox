#!/bin/env perl
######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################

$|++;
use strict;
use warnings;

my $worker = Gearbox::TestPerlWorker->new( $ARGV[0] );
$worker->run();

package Gearbox::TestPerlWorker;

use strict;
use warnings;

use Gearbox::Worker;
use base 'Gearbox::Worker';

sub new {
    my $class = shift;
    my $config = shift;

    my $self = $class->SUPER::new( $config );
    $self->register_handler( "do_get_testperl_example_v1", \&get_example);
    $self->register_handler( "do_post_testperl_example_v1" );
    
    return $self;
}

sub do_post_testperl_example_v1 {
    my($self,$job,$resp) = @_;
    my $status = $resp->status();
    $status->add_message("this is a status message!");
    die new ERR_INTERNAL_SERVER_ERROR("uh oh");
}

sub get_example {
    my($self,$job,$resp) = @_;
    print "content: ", $job->content(), "\n";
    print "serialize: ", $job->serialize(), "\n";
    
    my $args = $job->arguments();
    for( my $i = 0; $i < @$args; $i++ ) {
        print "Arg $i: ", $args->[$i], "\n";
    }
    my $key;

    my $matrix = $job->matrix_arguments();
    for $key ( keys %$matrix ) {
        print "Matrix Arg: $key => $matrix->{$key}\n";
    }

    my $params = $job->query_params();
    for $key ( keys %$params ) {
        print "Query Param: $key => $params->{$key}\n";
    }

    my $headers = $job->headers();
    for $key ( keys %$headers ) {
        print "Header: $key => $headers->{$key}\n";
    }

    my $environ = $job->environ();
    for $key ( keys %$environ ) {
        print "ENV: $key => $environ->{$key}\n";
    }
    
    print "status: " . $job->status() . "\n";
    print "name: " . $job->name() . "\n";
    print "base_uri: " . $job->base_uri() . "\n";
    
    print "type: ";
    my $type = $job->type();
    print 
        $type == $Gearbox::Job::JOB_ASYNC ? "ASYNC\n" :
        $type == $Gearbox::Job::JOB_SYNC  ? "SYNC\n"  : 
                                            "UNKNOWN\n";
    print "api_version: " . $job->api_version() . "\n";
    print "operation: " . $job->operation() . "\n";
    print "component: " . $job->component() . "\n";
    print "resource_type: " . $job->resource_type() . "\n";
    print "resource_name: " . $job->resource_name() . "\n";
    print "resource_uri: " . $job->resource_uri() . "\n";
    print "remote_ip: " . $job->remote_ip() . "\n";
    print "remote_user: " . $job->remote_user() . "\n";
    print "timeout: " . $job->timeout() . "\n";
    
    print "resp code: " . $resp->code() . "\n";
    
    my $status = $resp->status();
    $status->add_message("this is a status message!");
    print "status name: " . $status->name() . "\n";
    print "status resource_uri: " . $status->resource_uri() . "\n";
    print "status operation: " . $status->operation() . "\n";
    
    $resp->content('{"hello": "world"}');
    print "Done\n";
    return $WORKER_SUCCESS;
}
