#!/usr/bin/env perl
use lib $ENV{PWD}."/".($0 =~ m{(.*)/[^/]+$})[0];
use strict;
use warnings;
use setup;
use JSON qw(from_json);

use Gearbox::Logger;
Gearbox::Logger->init("./unit.conf");

use Test::Trivial (tests => 82);

NOTHROW { eval "use Gearbox::StatusManager" };

IS Gearbox::Status::state2str($Gearbox::Status::STATE_UNKNOWN) => "UNKNOWN";
IS Gearbox::Status::state2str($Gearbox::Status::STATE_PENDING) => "PENDING";
IS Gearbox::Status::state2str($Gearbox::Status::STATE_RUNNING) => "RUNNING";
IS Gearbox::Status::state2str($Gearbox::Status::STATE_STOPPING) => "STOPPING";
IS Gearbox::Status::state2str($Gearbox::Status::STATE_STOPPED) => "STOPPED";
IS Gearbox::Status::state2str($Gearbox::Status::STATE_CANCELLING) => "CANCELLING";
IS Gearbox::Status::state2str($Gearbox::Status::STATE_COMPLETED) => "COMPLETED";

IS Gearbox::Status::str2state("UNKNOWN") => $Gearbox::Status::STATE_UNKNOWN;
IS Gearbox::Status::str2state("PENDING") => $Gearbox::Status::STATE_PENDING;
IS Gearbox::Status::str2state("RUNNING") => $Gearbox::Status::STATE_RUNNING;
IS Gearbox::Status::str2state("STOPPING") => $Gearbox::Status::STATE_STOPPING;
IS Gearbox::Status::str2state("STOPPED") => $Gearbox::Status::STATE_STOPPED;
IS Gearbox::Status::str2state("CANCELLING") => $Gearbox::Status::STATE_CANCELLING;
IS Gearbox::Status::str2state("COMPLETED") => $Gearbox::Status::STATE_COMPLETED;

IS Gearbox::Status::event2str($Gearbox::Status::EVENT_UNKNOWN) => "UNKNOWN";
IS Gearbox::Status::event2str($Gearbox::Status::EVENT_PRECANCEL) => "PRECANCEL";
IS Gearbox::Status::event2str($Gearbox::Status::EVENT_CANCEL) => "CANCEL";

IS Gearbox::Status::str2event("UNKNOWN") => $Gearbox::Status::EVENT_UNKNOWN;
IS Gearbox::Status::str2event("PRECANCEL") => $Gearbox::Status::EVENT_PRECANCEL;
IS Gearbox::Status::str2event("CANCEL") => $Gearbox::Status::EVENT_CANCEL;

OK my $sm = Gearbox::StatusManager->new(Gearbox::ConfigFile->new("./unit.conf"));
my $BASE = "http://localhost:4080/transient/";
NOTHROW { $sm->base_uri($BASE) };

OK my $p = $sm->create("s-0", "operation", "$BASE/thing", "component");
OK my $s = $sm->create("s-1", "operation", "$BASE/thing", "component");
IS $s->name() => "s-1";
IS $s->operation() => "operation";
IS $s->resource_uri() => "$BASE/thing";
IS $s->component() => "component";
IS $s->uri() => "http://localhost:4080/transient/status/s-1";

IS $sm->fetch("s-1") => $s;
IS $sm->fetch(from_json($s->serialize())) => $s;

# setup up parent
NOTHROW { $s->parent_uri($p->uri()) };
IS $s->parent_uri() => $p->uri();
IS $s->parent()->serialize() => $p->serialize();

# test message apis
IS $s->messages() => [];
NOTHROW { $s->add_message("message") };
IS $s->messages() => ["message"];

# test children apis
IS $s->children() => [];
NOTHROW { $s->add_child("$BASE/status/s-2") };
IS $s->children() => ["$BASE/status/s-2"];

# test meta apis
IS $s->meta() => undef;
NOTHROW { $s->meta("key" => "value") };
IS $s->meta() => {key => "value"};
NOTHROW { $s->meta({a=>1, b=>2}) };
IS $s->meta() => {a=>1, b=>2};

# test progress apis
IS $s->progress() => 0;
NOTHROW { $s->progress(50) };
IS $s->progress() => 50;

# fail the status, set progress, code and state
NOTHROW { $s->fail(123) };
IS $s->progress() => 100;
IS $s->code() => 123;
IS $s->state => $Gearbox::Status::STATE_COMPLETED;

# set to successful, progress, code and state updated
NOTHROW { $s->success() };
IS $s->progress() => 100;
IS $s->code() => 0;
IS $s->state => $Gearbox::Status::STATE_COMPLETED;

NOTHROW { $s->checkpoint };

# test cancel api
NOTHROW { $s->state($Gearbox::Status::STATE_CANCELLING) };
NOTHROW { $s->cancel() };
IS $s->code() => 0;
IS $s->state() => $Gearbox::Status::STATE_CANCELLED;

IS THROWS { $s->checkpoint } => "Worker Stop";

NOTHROW { eval "use Gearbox::JobManager" };
OK my $jm = Gearbox::JobManager->new(Gearbox::ConfigFile->new("./unit.conf"));
OK my $job = $jm->job("do_get_component_other_v1");

NOTHROW { $s->on($Gearbox::Status::EVENT_PRECANCEL => $job) };
IS $s->on($Gearbox::Status::EVENT_PRECANCEL, $jm) => $job;

NOTHROW { $s->on($Gearbox::Status::EVENT_CANCEL => $job) };
IS $s->on($Gearbox::Status::EVENT_CANCEL, $jm) => $job;

OK $s->ctime() <= time();
OK $s->mtime() <= time();

IS $s->ytime() => -1;
NOTHROW { $s->ytime(0) };
IS $s->ytime() => 0;

NOTHROW { $s->sync() };

IS $s->has_completed => 1;
OK $s->is_success();

IS $s->failures() => 0;
NOTHROW { $s->failures(1) };
IS $s->failures() => 1;

IS from_json($s->serialize) => {
    'failures' => 1,
    'ctime' => $s->ctime,
    'messages' => [
        'message'
    ],
    'progress' => 100,
    'mtime' => $s->mtime,
    'children' => [
        'http://localhost:4080/transient/status/s-2'
    ],
    'uri' => 'http://localhost:4080/transient//thing',
    'state' => 'CANCELLED',
    'meta' => {
        'a' => 1,
        'b' => 2
    },
    'parent_uri' => 'http://localhost:4080/transient/status/s-0',
    'component' => 'component',
    'status_uri' => 'http://localhost:4080/transient/status/s-1',
    'operation' => 'operation',
    'code' => 0,
    'concurrency' => 0
};
