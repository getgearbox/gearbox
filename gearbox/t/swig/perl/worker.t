#!/usr/bin/env perl
use lib $ENV{PWD}."/".($0 =~ m{(.*)/[^/]+$})[0];
use strict;
use warnings;

package TestWorker;
use setup;
use JSON qw(to_json from_json -convert_blessed_universally);
use Test::Trivial (tests => 55);
use Gearbox::Worker;
use base qw(Gearbox::Worker);
sub new { shift->SUPER::new(@_) };

OK my $worker = TestWorker->new("./unit.conf");

OK my $jm = $worker->job_manager();
NOTHROW { $jm->base_uri("http://localhost:4080/test/v1") };

OK my $sm = $worker->status_manager();
NOTHROW { $sm->base_uri("http://localhost:4080/test/v1") };

# test contstants
IS $WORKER_SUCCESS  => 0;
IS $WORKER_ERROR    => 1;
IS $WORKER_CONTINUE => 2;
IS $WORKER_RETRY    => 3;

# verify 3 forms of register_handler 
# work.  Each handler will check api_version of 
# incoming job to make sure it matches our api

NOTHROW { $worker->register_handler(
    "do_run_handler_thing_v1",
    sub { 
        my ($self, $job, $resp) = @_;
        IS $job->api_version() => 'v1';
        return $WORKER_SUCCESS
    } 
) };
NOTHROW { $worker->register_handler( "do_run_handler_thing_v2", \&handler ) };
NOTHROW { $worker->register_handler( "do_run_handler_thing_v3") };

sub handler {
    my ($self, $job, $resp) = @_;
    IS $job->api_version() => 'v2';
    return $WORKER_SUCCESS;
}
sub do_run_handler_thing_v3 {
    my ($self, $job, $resp) = @_;
    IS $job->api_version() => 'v3';
    return $WORKER_SUCCESS;
}

# this is the order we will run the jobs so that we can test the names
# in the pre/post request handlers
my @queue = qw(do_run_handler_thing_v1 do_run_handler_thing_v2 do_run_handler_thing_v3);

# these get called automatically when a job comes in, verify the job
# name is correct and verify no exceptions from parent pre/post request handlers
sub pre_request {
    my ($self, $job) = @_;
    IS $job->name() => $queue[0];
    NOTHROW { $self->SUPER::pre_request($job) };
}

sub post_request {
    my ($self, $job) = @_;
    IS $job->name() => shift @queue;
    NOTHROW { $self->SUPER::post_request($job) };
}

# reset max_requests so that we can test the worker::run
# (otherwise it is an infinate loop)

NOTHROW { $worker->max_requests(1) };
IS $worker->request_count() => 0;

# create v1 job, add it to queue, then let worker run it
OK my $job = $jm->job("do_run_handler_thing_v1");
NOTHROW { $job->run() };
NOTHROW { $worker->run() };

# update to v2 job, add it to queue, then let worker run it
NOTHROW { $job->name("do_run_handler_thing_v2")->run() };
NOTHROW { $worker->run() };

# update to v3 job, add it to queue, then let worker run it
NOTHROW { $job->name("do_run_handler_thing_v3")->run() };
NOTHROW { $worker->run() };

# we have handled requests, count should be updated
IS $worker->request_count() => 3;

# verify aftewards jobs dont throw exceptions
NOTHROW { $worker->afterwards( $job ) };
NOTHROW { $worker->afterwards( $job, 30 ) };

OK my $cfg = $worker->cfg();
IS $cfg->get_string("component") => "unit";

#
# treat obj as hash, ie test tied hash accessors
#

OK $worker->{a} = 1;
IS $worker->{a} => 1;
OK $worker->{b} = 2;
IS $worker->{b} => 2;
my $itr="a";
for my $k ( keys %$worker ) {
    IS $k => $itr;
    IS $worker->{$k} => (ord($itr) - ord("a") + 1);
    $itr++;
}
$itr="a";
while( my ($k,$v) = each %$worker ) {
    IS $k => $itr;
    IS $v => (ord($itr) - ord("a") + 1);
    $itr++;
}

IS to_json($worker, { convert_blessed => 1 } ) => '{"a":1,"b":2}';
%$worker = ();
IS to_json($worker, { convert_blessed => 1 } ) => "{}";
