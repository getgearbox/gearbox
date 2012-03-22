#!/usr/bin/perl
use strict;
use warnings;
use lib qw(../../../perl/Gearbox_Rest_Client);
use Gearbox::Rest::Client;
use Test::Trivial tests => 118;

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

my $c = Gearbox::Rest::Client->new( trace => $TRACE );

my $PREFIX = $ENV{PREFIX} ? "/$ENV{PREFIX}" : "";
my $URI = "http://" . ($ENV{SMOKE_HOST} || "localhost:4080") . "$PREFIX/test-agents/v1";

my $data;
my $err;

######################################################################
#
# do an async POST where several other sub jobs are called async.
# each sub job has it own status which is visiable via
# children attribute in the status object
# The call graph should look something like this:
# do_post_testagents_thing
# -> do_run_global_agents
#    -> do_reg_testagents_A_v1
#    -> do_reg_testagents_B_v1
#    -> do_reg_testagents_C_v1
#    -> do_reg_testagents_D_v1
#
# both do_reg_testagents_B_v1 do_reg_testagents_C_v1 are run in 
# parallel so the execution and completion order will be random
# but do_reg_testagents_A_v1 will run before B and C
# and do_reg_testagents_D_v1 will run after B and C
# 
######################################################################
my $status;
{
    local $c->{'Gearbox::Rest::Client'}->{auto_poll} = 0
;

    OK $data = $c->req("POST", "$URI/thing", "{}");     
    my $resource_uri = $c->status()->{uri};
    my $status_uri = $c->status()->{status_uri};
    sleep(5);

    my $s = $c->req("GET", $c->status()->{status_uri});
    # first child should be the do_run_global_agents_v1 job
    my $run_status_uri = $s->{children}->[0];

    # progress should be 1% when do_run_global_agents_v1 has started
    # but the first level is not complete
    $s = $c->req("GET", $run_status_uri);
    IS $s->{progress} => 1;

    sleep(10);
    # progress should be 33% when first nth degree has finished
    # (there are 3 nth degrees: A, B+C, D)
    $s = $c->req("GET", $run_status_uri);
    IS $s->{progress} => 33;
    
    sleep(10);
    # progress should be 66% when second nth degree has finished
    # (there are 3 nth degrees: A, B+C, D)
    $s = $c->req("GET", $run_status_uri);
    IS $s->{progress} => 66;


    sleep(10);
    # progress should be 100% when third/final nth degree has finished
    # (there are 3 nth degrees: A, B+C, D)
    $s = $c->req("GET", $run_status_uri);
    IS $s->{progress} => 100;

    OK $status = $c->req("GET", $status_uri);
    OK $data = $c->req("GET", $resource_uri);
}

my $id = $data->{id};
LIKE $id, qr/t-\S+/;

IS $status->{messages}, ["calling agents", "done"];

IS $status->{progress} => 100;
IS $status->{state}   => "COMPLETED";
IS $status->{operation} => "create";
IS $status->{code} => 0;
IS $status->{parent_uri} => undef;

my $create_status_uri = $status->{status_uri};

# should have 1 child for the do_run_global_agents
LIKE $status->{children}->[0] => qr{$URI/status/s-\S};
# fetch the agents status so we can verify it
OK my $agents = $c->req("GET", $status->{children}->[0]);

IS $agents->{progress} => 100;
IS $agents->{state}   => "COMPLETED";
IS $agents->{operation} => "run";
IS $agents->{code} => 0;
IS $agents->{parent_uri} => $create_status_uri;

# should be 4 child statuses
LIKE $agents->{children}->[0] => qr{$URI/status/s-\S+};
LIKE $agents->{children}->[1] => qr{$URI/status/s-\S+};
LIKE $agents->{children}->[2] => qr{$URI/status/s-\S+};
LIKE $agents->{children}->[3] => qr{$URI/status/s-\S+};
IS @{$agents->{children}}, 4;

# check the child statuses
my @status_uris = @{$agents->{children}};

OK my $A = $c->req("GET", $status_uris[0]);
IS $A->{messages} => ["A registered for $id"];
IS $A->{progress} => 100;
IS $A->{children} => [];
IS $A->{state}   => "COMPLETED";
IS $A->{operation} => "reg";
IS $A->{code} => 0;
IS scalar keys %{ $A->{meta} } => 1;
my ($status_id) = ($status_uris[0] =~ m#/status/(s-\S+)#);
IS $A->{meta}->{$status_id} => $id;
IS $A->{parent_uri} => $agents->{status_uri};

OK my $BorC1 = $c->req("GET", $status_uris[1]);
LIKE $BorC1->{messages}[0] => qr{(B|C) registered for $id};
IS $BorC1->{progress} => 100;
IS $BorC1->{children} => [];
IS $BorC1->{state}   => "COMPLETED";
IS $BorC1->{operation} => "reg";
IS $BorC1->{code} => 0;
IS scalar keys %{ $BorC1->{meta} } => 1;
($status_id) = ($status_uris[1] =~ m#/status/(s-\S+)#);
IS $BorC1->{meta}->{$status_id} => $id;
IS $BorC1->{parent_uri} => $agents->{status_uri};

OK my $BorC2 = $c->req("GET", $status_uris[2]);
LIKE $BorC1->{messages}[0] => qr{(B|C) registered for $id};
IS $BorC2->{progress} => 100;
IS $BorC2->{children} => [];
IS $BorC2->{state}   => "COMPLETED";
IS $BorC2->{operation} => "reg";
IS $BorC2->{code} => 0;
IS scalar keys %{ $BorC2->{meta} } => 1;
($status_id) = ($status_uris[2] =~ m#/status/(s-\S+)#);
IS $BorC2->{meta}->{$status_id} => $id;
IS $BorC2->{parent_uri} => $agents->{status_uri};

OK my $D = $c->req("GET", $status_uris[3]);
IS $D->{messages} => ["D registered for $id"];
IS $D->{progress} => 100;
IS $D->{children} => [];
IS $D->{state}   => "COMPLETED";
IS $D->{operation} => "reg";
IS $D->{code} => 0;
IS scalar keys %{ $D->{meta} } => 1;
($status_id) = ($status_uris[3] =~ m#/status/(s-\S+)#);
IS $D->{meta}->{$status_id} => $id;
IS $D->{parent_uri} => $agents->{status_uri};

# verify the tree with start/end times
# mtimes will be the last update (finalization time)
# and ctime is when created

OK $A->{mtime} <= $BorC1->{ctime};
OK $A->{mtime} <= $BorC2->{ctime};
OK $BorC1->{mtime} <= $D->{ctime};
OK $BorC2->{mtime} <= $D->{ctime};

######################################################################
#
# do an async DELETE where 2 other sub jobs are called async.
# each sub job has it own status which is visiable via
# children attribute in the status object
#
######################################################################
OK $data = $c->req("DELETE", "$URI/thing/$id");

IS $c->status()->{messages}, ["calling agents", "done"];

IS $c->status()->{progress} => 100;
LIKE $c->status()->{children}->[0] => qr{$URI/status/s-\S};
LIKE $c->status()->{children}->[1] => qr{$URI/status/s-\S};
LIKE $c->status()->{children}->[2] => qr{$URI/status/s-\S};
LIKE $c->status()->{children}->[3] => qr{$URI/status/s-\S};
IS @{$c->status()->{children}}, 4;
IS $c->status()->{state}   => "COMPLETED";
IS $c->status()->{operation} => "delete";
IS $c->status()->{code} => 0;
IS $c->status()->{parent_uri} => undef;

# check the child statuses
@status_uris = @{$c->status()->{children}};

my $delete_status_uri = $c->status()->{status_uri};

OK $D = $c->req("GET", $status_uris[0]);
IS $D->{messages} => ["D unregistered for $id"];
IS $D->{progress} => 100;
IS $D->{children} => [];
IS $D->{state}   => "COMPLETED";
IS $D->{operation} => "unreg";
IS $D->{code} => 0;
IS $D->{parent_uri} => $delete_status_uri;


OK $BorC1 = $c->req("GET", $status_uris[1]);
LIKE $BorC1->{messages}[0] => qr{(B|C) unregistered for $id};
IS $BorC1->{progress} => 100;
IS $BorC1->{children} => [];
IS $BorC1->{state}   => "COMPLETED";
IS $BorC1->{operation} => "unreg";
IS $BorC1->{code} => 0;
IS $BorC1->{parent_uri} => $delete_status_uri;

OK $BorC2 = $c->req("GET", $status_uris[2]);
LIKE $BorC1->{messages}[0] => qr{(B|C) unregistered for $id};
IS $BorC2->{progress} => 100;
IS $BorC2->{children} => [];
IS $BorC2->{state}   => "COMPLETED";
IS $BorC2->{operation} => "unreg";
IS $BorC2->{code} => 0;
IS $BorC2->{parent_uri} => $delete_status_uri;

OK $A = $c->req("GET", $status_uris[3]);
IS $A->{messages} => ["A unregistered for $id"];
IS $A->{progress} => 100;
IS $A->{children} => [];
IS $A->{state}   => "COMPLETED";
IS $A->{operation} => "unreg";
IS $A->{code} => 0;
IS $A->{parent_uri} => $delete_status_uri;

# verify the tree with start/end times

OK $D->{mtime} <= $BorC1->{ctime};
OK $D->{mtime} <= $BorC2->{ctime};
OK $BorC1->{mtime} <= $A->{ctime};
OK $BorC2->{mtime} <= $A->{ctime};
