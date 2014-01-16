#!/usr/bin/env perl
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

my $worker = Gearbox::WorkerTestCancelPerl->new( $ARGV[0] );
$worker->run();

package Gearbox::WorkerTestCancelPerl;

use strict;
use warnings;

use Gearbox::Worker;
use base 'Gearbox::Worker';

sub new {
    my $class = shift;
    my $config = shift;

    my $self = $class->SUPER::new( $config );

    $self->register_handler( "do_post_testcancelperl_thing_v1" );
    $self->register_handler( "do_cancel_testcancelperl_thing_v1" );

    $self->register_handler( "do_post_testcancelperl_continuation_v1" );
    $self->register_handler( "do_run_testcancelperl_continuation_v1" );
    $self->register_handler( "do_finish_testcancelperl_continuation_v1" );
    return $self;
}

sub do_post_testcancelperl_thing_v1 {
    my $self = shift;
    my $job  = shift;
    my $resp = shift;
    my $onCancel = $self->job_manager()->job("do_cancel_testcancelperl_thing_v1");
    $resp->status()->on($Gearbox::Status::EVENT_CANCEL, $onCancel);

    my $s = $resp->status();
    my $stop = time() + 30;
    while ( $stop >= time() ) {
        $s->sync();
        my $p = $s->progress();
        if ( $p < 100 ) {
            $p += 10;
            $s->progress( $p );
            $s->checkpoint();
            sleep(5);
        } else {
            return $WORKER_SUCCESS;
        }
    }
}

sub do_cancel_testcancelperl_thing_v1 {
    my ($self,$job,$resp) = @_;
    $resp->status()->add_message("on cancel callback called");
    return $WORKER_SUCCESS;
}

sub do_post_testcancelperl_continuation_v1 {
    my ($self,$job,$resp) = @_;
    # we have do_post with a continuation of do_finish.  The do_finish
    # is only called via on-completion handler for do_run
    my $run = $self->job_manager()->job("do_run_testcancelperl_continuation_v1");
    my $finish = Gearbox::Job->new($job);
    $finish->name("do_finish_testcancelperl_continuation_v1");
    $run->on($Gearbox::Job::EVENT_COMPLETED, $finish);
    $self->afterwards($run);
    return $WORKER_CONTINUE;
}
sub do_run_testcancelperl_continuation_v1 {
    my ($self,$job,$resp) = @_;
    $resp->status()->add_message("run called");
    # this will retry indefinately, we want to test the cancellation of an
    # status in progress that is suspended waiting upon child completion events
    return $WORKER_RETRY;
}
sub do_finish_testcancelperl_continuation_v1 {
    my ($self,$job,$resp) = @_;
    # this will never get called since do_run will never complete
    return $WORKER_SUCCESS;
}
1;
