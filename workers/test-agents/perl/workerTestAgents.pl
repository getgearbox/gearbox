#!/bin/env perl
######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################

TestAgentsWorker->new($ARGV[0])->run();

package TestAgentsWorker;
use strict;
use warnings;
use Gearbox::Worker;
use base qw(Gearbox::Worker);
use JSON;

use constant DBDIR => "/var/gearbox/db/test-agents-perl/";

sub new {
    my $self = shift->SUPER::new(@_);
    
    $self->register_handler( "do_get_testagentsperl_thing_v1", \&thing_handler );
    $self->register_handler( "do_post_testagentsperl_thing_v1", \&thing_handler);
    $self->register_handler( "do_delete_testagentsperl_thing_v1", \&thing_handler);
    $self->register_handler( "do_reg_testagentsperl_A_v1", \&dummy_handler);
    $self->register_handler( "do_unreg_testagentsperl_A_v1", \&dummy_handler);
    $self->register_handler( "do_reg_testagentsperl_B_v1", \&dummy_handler);
    $self->register_handler( "do_unreg_testagentsperl_B_v1", \&dummy_handler);
    $self->register_handler( "do_reg_testagentsperl_C_v1", \&dummy_handler);
    $self->register_handler( "do_unreg_testagentsperl_C_v1", \&dummy_handler);
    $self->register_handler( "do_reg_testagentsperl_D_v1", \&dummy_handler);
    $self->register_handler( "do_unreg_testagentsperl_D_v1", \&dummy_handler);
    
    return $self;
}

my $json;
sub json {
    return $json || ($json = JSON->new->allow_nonref);
}

sub thing_handler {
    my ($self, $job, $resp) = @_;
    my $content = {};
    if( -f DBDIR . $job->resource_name() ) { 
        $content = $self->json()->decode(slurp( DBDIR . $job->resource_name() ));
    }

    if( $job->operation() eq "get" ) {
      $resp->content($self->json()->encode($content));
      return $WORKER_SUCCESS;
    }

    my $agents = $self->json()->decode(slurp("/etc/gearbox/test-agents-perl-agents.conf"));

    $resp->status()->add_message("calling agents");

    if( $job->operation() eq "create" ) {
        $content->{"id"} = $job->resource_name();
        write_file(DBDIR.$job->resource_name(), $self->json()->encode($content));

        my $run_agents = $self->job_manager()->job("do_run_global_agents_v1");
        my $agents_content = {};
        $agents_content->{"agents"} = $agents->{"register"};
        $agents_content->{"content"} = $self->json()->encode($content);
        $run_agents->content( $self->json()->encode($agents_content) );
        my $r = $run_agents->run();
        my $s = $r->status();
        # poll for agents to be done
        do {
            sleep(1);
            $s->sync();
        } while( ! $s->has_completed() );
        
        if ( ! $s->is_success() ) {
            my $ERR = "ERR_CODE_" . $s->code();
            die $ERR->new( $s->messages()->[-1] );
        }
    }
    else {
        # opertaion == delete
        my $jm = $self->job_manager();
        my $queue = $jm->job_queue( $agents->{"unregister"} );
        $jm->job_queue_apply($queue, \&Gearbox::Job::content, $self->json()->encode($content));
        $jm->job_queue_run($queue);
        unlink( DBDIR . $job->resource_name() );
    }
    $resp->status()->add_message("done");
    return $WORKER_SUCCESS;
}

sub dummy_handler {
    my ($self,$job, $resp) = @_;
    my $in = $self->json()->decode($job->content());
    my $msg  = $job->resource_type() . " ";
    $msg .= $job->operation() eq "reg" ? "registered" : "unregistered";
    $msg .= " for " . $in->{"id"};
    $resp->status()->add_message($msg);
    # give us time from smoke tests to verify the progress of the
    # agents job
    if( $job->operation() eq "reg" ) {
        sleep(10);
    }

    $resp->status()->meta( $resp->status()->name(), $in->{"id"} );
    return $WORKER_SUCCESS;
}
 
#
# Helper Routines
#

sub slurp {
    my $file = shift;
    return do {
        local $/ = undef;
        open my $fh, '<', $file or die "can't read contents of $file: $!\n";
        <$fh>;
    };
}

sub write_file {
    my $file = shift;
    my $contents = shift;
    open my $fh, ">", $file or die "Couldn't open $file to write: $!\n";
    print $fh $contents;
    close $fh;
    return;
}

1;
