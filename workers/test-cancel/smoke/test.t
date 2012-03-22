#!/usr/bin/perl
use strict;
use warnings;
use lib qw(../../../perl/Gearbox_Rest_Client);
use Gearbox::Rest::Client;
use Test::Trivial tests => 50;

use Log::Log4perl qw(:easy);
Log::Log4perl->easy_init({
    level    => $DEBUG,
    file     => "STDOUT",
    layout   => '# %m%n'
});

use Getopt::Long;
my $TRACE = 0;
GetOptions(
    'trace' => \$TRACE,
);

my $c = Gearbox::Rest::Client->new(
   trace => $TRACE,
   auto_poll => 0
);
my $PREFIX = $ENV{PREFIX} ? "/$ENV{PREFIX}" : "";
my $URI = "http://" . ($ENV{SMOKE_HOST} || "localhost:4080") . "$PREFIX/test-cancel/v1";

my $data;
my $err;

######################################################################
#
# start a job through the REST api.
#
######################################################################
OK $data = $c->req("POST", "$URI/thing/foo", '{}');

# check the polled status to make sure it looks normal
IS $c->status()->{progress} => 0;
IS $c->status()->{uri} => "$URI/thing/foo";
IS $c->status()->{state} => "PENDING";
IS $c->status()->{operation} => "update";

my $status_uri = $c->status()->{status_uri};
Test::More::diag($status_uri);
sleep 1;
OK $data = $c->req("GET", $status_uri );
OK $data->{progress} > 0;
IS $data->{state} => "RUNNING";

OK $data = $c->req("POST", $status_uri, '{"state": "CANCELLED"}');
IS $data->{operation} => "update";
IS $data->{uri} => $status_uri;

sleep 1;
OK $data = $c->req("GET", $status_uri );
LIKE $data->{state} => qr{^STOPPING|STOPPED|CANCELLING$};

sleep 25;
OK $data = $c->req("GET", $status_uri );
IS $data->{progress} => 100;
IS $data->{state} => "CANCELLED";
IS @{$data->{children}} => 1;

OK my $child = $c->req("GET", $data->{children}->[0] );
IS $child->{progress} => 100;
IS $child->{state} => "COMPLETED";
IS $child->{messages}->[0] => "on cancel callback called";
IS $child->{operation} => "cancel";



######################################################################
#
# start a contination job that will never finish, so that we can 
# cancel a contination in progress
#
######################################################################
OK $data = $c->req("POST", "$URI/continuation", '{}');
my $parent_status = $c->status()->{status_uri};

# arbitrary time to get jobs running in queue and have
# child job start running
Test::More::diag("sleeping");
sleep 5;

$data = $c->req("GET", $parent_status);
IS $data->{progress} => 0;
IS $data->{state} => "RUNNING";
IS $data->{ytime} => undef;
IS @{$data->{children}}, 1;

my $child_status = $data->{children}->[0];

$child = $c->req("GET", $child_status);
IS $child->{progress}, 0;
# it will never end, so will either be in delay queue
# or it will be running
IS $child->{state} => "RUNNING";

######################################################################
#
# Cancel never-ending job.  The parent should go from STOPPING to
# STOPPED, then the child should go from STOPPING, STOPPED to CANCELLED
#
######################################################################

OK $data = $c->req("POST", $parent_status, '{"state":"CANCELLED"}');
my $cancel_status = $c->status()->{status_uri};

my $pstate = $c->req("GET", $parent_status)->{state};
my $cstate = $c->req("GET", $child_status)->{state};

Test::More::diag("parent: $pstate");
Test::More::diag("child: $cstate");

while(1) {
    my $new_pstate = $c->req("GET", $parent_status)->{state};
    my $new_cstate = $c->req("GET", $child_status)->{state};

    if( $pstate ne $new_pstate ) {
        $pstate = $new_pstate;
        Test::More::diag("parent: $pstate");
    }

    if( $cstate ne $new_cstate ) {
        $cstate = $new_cstate;
        Test::More::diag("child: $cstate");
    }

    $data = $c->req("GET", $cancel_status);
    if( $data->{progress} == 100 ) {
        last;
    }
    sleep 5;
}

IS $c->req("GET", $parent_status)->{state}  => "CANCELLED";
IS $c->req("GET", $child_status)->{state}  => "CANCELLED";
OK $data = $c->req("GET", $cancel_status);
IS $data->{progress} => 100;
IS $data->{state} => "COMPLETED";

######################################################################
#
# start a contination job that will never finish, so that we can 
# cancel a contination in progress, this time we are going
# to cancel the child job and verify that the on-complete event
# gets called to complete the parent status
#
######################################################################
OK $data = $c->req("POST", "$URI/continuation", '{}');
$parent_status = $c->status()->{status_uri};

# arbitrary time to get jobs running in queue and have
# child job start running
Test::More::diag("sleeping");
sleep 15;

$data = $c->req("GET", $parent_status);
IS $data->{progress} => 0;
IS $data->{state} => "RUNNING";
IS $data->{ytime} => undef;
IS @{$data->{children}}, 1;

$child_status = $data->{children}->[0];

$child = $c->req("GET", $child_status);
IS $child->{progress}, 0;
# it will never end, so will either be in delay queue
# or it will be running
IS $child->{state} => "RUNNING";

######################################################################
#
# Cancel never-ending job.  The child should go from STOPPING, STOPPED to CANCELLED
# and the parent should go from RUNNING to COMPLETED
#
######################################################################

OK $data = $c->req("POST", $child_status, '{"state":"CANCELLED"}');
$cancel_status = $c->status()->{status_uri};

$pstate = $c->req("GET", $parent_status)->{state};
$cstate = $c->req("GET", $child_status)->{state};

Test::More::diag("parent: $pstate");
Test::More::diag("child: $cstate");

while(1) {
    my $pdata = $c->req("GET", $parent_status);
    my $new_pstate = $pdata->{state};
    my $new_cstate = $c->req("GET", $child_status)->{state};

    if( $pstate ne $new_pstate ) {
        $pstate = $new_pstate;
        Test::More::diag("parent: $pstate");
    }

    if( $cstate ne $new_cstate ) {
        $cstate = $new_cstate;
        Test::More::diag("child: $cstate");
    }

    my $cdata = $c->req("GET", $cancel_status);
    if( $pdata->{progress} == 100 && $cdata->{progress} == 100 ) {
        last;
    }
    sleep 5;
}

OK $data = $c->req("GET", $parent_status);
IS $data->{progress} => 100;
IS $data->{state} => "COMPLETED";

IS $c->req("GET", $child_status)->{state}  => "CANCELLED";

OK $data = $c->req("GET", $cancel_status);
IS $data->{progress} => 100;
IS $data->{state} => "COMPLETED";
