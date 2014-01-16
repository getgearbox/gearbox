#!/usr/bin/env perl
######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################

TestSyncWorker->new($ARGV[0])->run();

package TestSyncWorker;
use strict;
use warnings;
use Gearbox::Worker;
use base qw(Gearbox::Worker);
use JSON qw(to_json from_json);
use constant DBDIR => "/var/gearbox/db/test-chained-perl/";

sub new {
    my $self = shift->SUPER::new(@_);
    
    $self->register_handler( "do_get_testchainedperl_hello_v1" );
    $self->register_handler( "do_get_internalperl_hello1_v1" );
    $self->register_handler( "do_post_testchainedperl_hello2_v1" );
    
    $self->register_handler( "do_get_testchainedperl_goodbye_v1" );
    $self->register_handler( "do_post_testchainedperl_goodbye_v1" );
    $self->register_handler( "do_append_internalperl_goodbye1_v1" );
    $self->register_handler( "do_append_internalperl_goodbye2_v1" );
    
    $self->register_handler( "do_get_testchainedperl_thing_v1" );
    $self->register_handler( "do_post_testchainedperl_thing_v1" );
    $self->register_handler( "do_reg_internalperl_service1_v1" );
    $self->register_handler( "do_post_testchainedperl_service2_v1" );
    
    $self->register_handler( "do_delete_testchainedperl_thing_v1" );
    $self->register_handler( "do_unreg_internalperl_service1_v1" );
    $self->register_handler( "do_delete_testchainedperl_service2_v1" );

    return $self;
}

my $json;
sub json {
    return $json || ($json = JSON->new->allow_nonref);
}

# I am not sure why we would want to do a chained syncronous get, but 
# you can chain a bunch of sync jobs together
sub do_get_testchainedperl_hello_v1 {
    my($self,$job,$resp) = @_;

    my $content = $self->json()->encode("Hello from job");
    
    # do internal hello1 which just appends it name to our content
    my $j = new Gearbox::Job($job);
    $j->name("do_get_internalperl_hello1_v1");
    $j->type($Gearbox::Job::JOB_SYNC);
    $j->content($content);
    my $r = $j->run();
    
    # create sync http rest job back to localhost which takes
    # the output from previous job and adds its own name.
    $j = $self->job_manager()->job($Gearbox::HttpClient::METHOD_POST, $job->base_uri()."/hello2");
    $j->content($r->content());
    $j->headers($r->headers());
    $r = $j->run();
    
    $resp->content( $r->content() );
    $resp->headers( $r->headers() );
    return $WORKER_SUCCESS;
}

sub do_get_internalperl_hello1_v1 {
    my ( $self, $job, $resp )  = @_;
    my $in = $self->json()->decode($job->content());
    $in .= " and job1";
    $resp->add_header("job1-header", "1");
    $resp->content($self->json()->encode($in));
    return $WORKER_SUCCESS;
}
  
# this is a SYNC post call configured via the httpd-test-chained.conf
sub do_post_testchainedperl_hello2_v1 {
    my( $self, $job, $resp ) = @_;
    my $in = $self->json()->decode($job->content());
    $in .= " and job2";
    $resp->headers($job->headers());
    $resp->add_header("job2-header", "1");
    $resp->content($self->json()->encode($in));
    return $WORKER_SUCCESS;
}
  
sub do_get_testchainedperl_goodbye_v1 {
    my ( $self, $job, $resp ) = @_;
    $resp->content( slurp(DBDIR . $job->resource_name()) );
    return $WORKER_SUCCESS;
}

sub do_post_testchainedperl_goodbye_v1 {
    my ( $self, $job, $resp ) = @_;
    $resp->status()->add_message("processing from " . $job->name());
    my $content = $self->json()->encode("Goodbye from job");
    write_file( DBDIR . $job->resource_name(), $content );
    
    # do internal goodbye1 which just appends it name to our content
    _WARN( "About to schedule job for do_append_internalperl_goodbye1_v1");
    $self->afterwards($job, "do_append_internalperl_goodbye1_v1");
    _WARN("DONE");
    # don't finalize the status, are going to keep going
    return $WORKER_CONTINUE;
}

sub do_append_internalperl_goodbye1_v1 {
    my ( $self, $job, $resp ) = @_;
    $resp->status()->add_message("processing from " . $job->name());
    my $content = $self->json()->decode(slurp( DBDIR . $job->resource_name() ));
    $content .= " and job1";
    write_file( DBDIR . $job->resource_name(), $self->json()->encode($content) );
        
    # do internal goodbye2 which just appends it name to our content
    $self->afterwards($job, "do_append_internalperl_goodbye2_v1");
    # don't finalize the status, are going to keep going
    return $WORKER_CONTINUE;
}

sub do_append_internalperl_goodbye2_v1 {
    my ( $self, $job, $resp ) = @_;
    $resp->status()->add_message("processing from " . $job->name());
    my $content = $self->json()->decode( slurp( DBDIR . $job->resource_name() ) );
    $content .= " and job2";
    write_file( DBDIR . $job->resource_name(), $self->json()->encode($content) );
    
    # finally done so dont continue
    return $WORKER_SUCCESS;
}
  
sub do_get_testchainedperl_thing_v1 {
    my ( $self, $job, $resp ) = @_;
    $resp->content( slurp( DBDIR . $job->resource_name() ) );
    return $WORKER_SUCCESS;
}

sub do_post_testchainedperl_thing_v1 {
    my ( $self, $job, $resp ) = @_;
    $resp->status()->add_message("processing from " . $job->name());
    my $out = {};
    $out->{"id"} = $job->resource_name();
    write_file( DBDIR . $job->resource_name(), $self->json()->encode($out) );

    # our new thing needs to be registered with 2 fancy
    # services.  They can both be registered at the same 
    # time in parallel.
    my $jm = $self->job_manager();
    
    my @responses;
    
    # service 1 is registered via async local worker
    push @responses, $jm->job("do_reg_internalperl_service1_v1")->content($self->json()->encode($out))->run();
    
    push(@responses, $jm->job($Gearbox::HttpClient::METHOD_POST, $job->base_uri() . "/service2")->content($self->json()->encode($out))->run());
    
    while ( @responses ) {
        my $s = $responses[0]->status();
        $s->sync();
        if( $s->has_completed() ) {
            if( $s->is_success() ) {
                shift @responses;
            }
            else {
                my $ERR = "ERR_CODE_".$s->code();
                my $msgs = $s->messages();
                throw $ERR->new($msgs->[0]);
            }
            # pause between polling again
            sleep(1);
        }
    }
    return $WORKER_SUCCESS;
}

sub do_reg_internalperl_service1_v1 {
    my ($self, $job, $resp) = @_;
    $resp->status()->add_message("service1 registered");
    return $WORKER_SUCCESS;
}

sub do_post_testchainedperl_service2_v1 {
    my ($self, $job, $resp) = @_;
    $resp->status()->add_message("service2 registered");
    return $WORKER_SUCCESS;
}

sub do_delete_testchainedperl_thing_v1 {
    my ( $self, $job, $resp ) = @_;
    # our new thing needs to be unregistered with 2 fancy
    # services.  service 1 must be unregistered before service 2
    
    my $content = slurp( DBDIR . $job->resource_name() );
    
    my $jm = $self->job_manager();
    
    my $jobs = [];
    
    # first gen jobs only has service1, unregister happens via local worker
    $jobs->[0]->[0] = $jm->job("do_unreg_internalperl_service1_v1");
    
    # second gen jobs only has service 2, unregister happens via DELETE
    # http call on remote worker
    $jobs->[1]->[0] = $jm->job($Gearbox::HttpClient::METHOD_DELETE, $job->base_uri() . "/service2");
    
    $jm->job_queue_apply($jobs, \&Gearbox::Job::content, $content);
    $jm->job_queue_run($jobs);
    return $WORKER_SUCCESS;
}

sub do_unreg_internalperl_service1_v1 {
    my ($self, $job, $resp) = @_;
    $resp->status()->add_message("service1 unregistered");
    return $WORKER_SUCCESS;
}

sub do_delete_testchainedperl_service2_v1 {
    my ($self, $job, $resp) = @_;
    $resp->status()->add_message("service2 unregistered");
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
