#!/usr/bin/env perl
use lib $ENV{PWD}."/".($0 =~ m{(.*)/[^/]+$})[0];
use strict;
use warnings;
use setup;
use Test::Trivial (tests => 101);
use JSON qw(from_json);

use Gearbox;
use Gearbox::JobManager;
use Gearbox::Logger;

OK my $cfg = Gearbox::ConfigFile->new("./unit.conf");

Gearbox::Logger->init("./unit.conf");

# test job type constants
IS $Gearbox::Job::JOB_UNKNOWN => 0;
IS $Gearbox::Job::JOB_SYNC => 1;
IS $Gearbox::Job::JOB_ASYNC => 2;

# type job event constants
IS Gearbox::Job::event2str($Gearbox::Job::EVENT_UNKNOWN) => "UNKNOWN";
IS Gearbox::Job::event2str($Gearbox::Job::EVENT_COMPLETED) => "COMPLETED";
IS Gearbox::Job::event2str($Gearbox::Job::EVENT_FAILED) => "FAILED";
IS Gearbox::Job::event2str($Gearbox::Job::EVENT_SUCCEEDED) => "SUCCEEDED";
IS Gearbox::Job::event2str($Gearbox::Job::EVENT_STARTED) => "STARTED";
IS Gearbox::Job::event2str($Gearbox::Job::EVENT_STOPPED) => "STOPPED";
IS Gearbox::Job::event2str($Gearbox::Job::EVENT_CANCELLED) => "CANCELLED";

IS Gearbox::Job::str2event("UNKNOWN") => $Gearbox::Job::EVENT_UNKNOWN;
IS Gearbox::Job::str2event("COMPLETED") => $Gearbox::Job::EVENT_COMPLETED;
IS Gearbox::Job::str2event("FAILED") => $Gearbox::Job::EVENT_FAILED;
IS Gearbox::Job::str2event("SUCCEEDED") => $Gearbox::Job::EVENT_SUCCEEDED;
IS Gearbox::Job::str2event("STARTED") => $Gearbox::Job::EVENT_STARTED;
IS Gearbox::Job::str2event("STOPPED") => $Gearbox::Job::EVENT_STOPPED;
IS Gearbox::Job::str2event("CANCELLED") => $Gearbox::Job::EVENT_CANCELLED;

OK my $jm = Gearbox::JobManager->new($cfg);
$jm->base_uri("http://localhost:4080/component/v1");
$jm->parent_uri("http://localhost:4080/component/v1/status/s-12345");

NOK $jm->known_job_name("do_foobar");
OK $jm->known_job_name("do_get_component_resource_v1");
OK $jm->known_job_name("do_get_component_other_v1");

my $job;
# test jobs without schemas
$jm->requireSchemas(1);
NOTHROW { $job = $jm->job("do_get_component_other_v1") };
IS THROWS { $job->run() }  => qq(Required schema for job do_get_component_other_v1 not found!);

$jm->requireSchemas(0);
OK $job = $jm->job("do_get_component_other_v1");

# test jobs with schemas
$jm->requireSchemas(1);
OK $job = $jm->job("do_get_component_resource_v1");
NOTHROW { $job->content('{"key": "value"}') };
IS THROWS { $job->run() } => qq(Json Exception: invalid property "key": schema does not allow for this property);

# this should work with the schema
$job->content('{"this":"that"}');
IS $job->json_content(), { this => "that" };

# turn of schema requirements for futher testing
$jm->requireSchemas(0);

OK $job = $jm->job($Gearbox::HttpClient::METHOD_GET, "http://localhost:4080/component/v1/resource");

# test copy ctor
OK my $copy = Gearbox::Job->new($job);
IS $job->serialize() => $copy->serialize();
# delete job, verify that copy is still good
my $envelope = $job->serialize();
undef $job;
IS $copy->serialize() => $envelope;

# test delay/retry
NOTHROW { $jm->delay($copy, 10) };
NOTHROW { $jm->retry($copy) };

# test gen_id
LIKE $jm->gen_id('r') => qr{^r-[a-z0-9]+$};

# test all job accessors:
OK $job = $jm->job("do_get_component_resource_v1");
NOTHROW { $job->content('{"this":"that"}') };
IS $job->json_content() => { "this" => "that" };

NOTHROW { $job->add_argument("arg1") };
NOTHROW { $job->add_argument("arg2") };
IS $job->arguments(), ["arg1", "arg2"];

NOTHROW { $job->add_matrix_argument("arg1" => "val1") };
NOTHROW { $job->add_matrix_argument("arg2" => "val2") };
IS $job->matrix_arguments() => {arg1 => "val1", arg2 => "val2"};

NOTHROW { $job->add_query_param("arg1" => "val1") };
NOTHROW { $job->add_query_param("arg2" => "val2") };
IS $job->query_params() => {arg1 => "val1", arg2 => "val2"};

NOTHROW { $job->add_header("head1" => "val1") };
NOTHROW { $job->add_header("head2" => "val2") };
IS $job->headers() => {head1 => "val1", head2 => "val2"};

NOTHROW { $job->add_environ("env1" => "val1") };
NOTHROW { $job->add_environ("env2" => "val2") };
IS $job->environ() => {env1 => "val1", env2 => "val2"};

IS $job->status() => "";
IS $job->base_uri() => "http://localhost:4080/component/v1";
IS $job->name() => "do_get_component_resource_v1";
IS $job->api_version() => "v1";
IS $job->operation() => "get";
IS $job->component() => "component";
IS $job->resource_type() => "resource";

my $id = $jm->gen_id('r');
NOTHROW { $job->resource_name( $id ) };
IS $job->resource_name() => $id;

IS $job->type() => $Gearbox::Job::JOB_ASYNC;
NOTHROW { $job->type($Gearbox::Job::JOB_SYNC) };
IS $job->type() => $Gearbox::Job::JOB_SYNC;

IS $job->resource_uri() => "http://localhost:4080/component/v1/resource/$id";

NOTHROW { $job->remote_ip("1.2.3.4") };
IS $job->remote_ip() => "1.2.3.4";

NOTHROW { $job->remote_user("user") };
IS $job->remote_user() => "user";

# test on(EVENT) apis
IS $job->on($Gearbox::Job::EVENT_COMPLETED) => undef;

my $other = $jm->job("do_get_component_other_v1");
NOTHROW { $job->on($Gearbox::Job::EVENT_COMPLETED, $other) };
IS from_json( $job->on($Gearbox::Job::EVENT_COMPLETED)->serialize() ) => {
    arguments => [],
    matrix_arguments => {},
    query_params => {},
    environ => {},
    headers => {},
    operation => "get",
    component => "component",
    version => "v1",
    job_type => "async",
    resource => { type => "other" },
    base_uri => "http://localhost:4080/component/v1",
    parent_uri => "http://localhost:4080/component/v1/status/s-12345",
};

IS from_json( $job->serialize() ) => {
    content => '{"this":"that"}',
    arguments => ["arg1", "arg2"],
    matrix_arguments => {"arg1" => "val1", "arg2" => "val2"},
    query_params => { "arg1" => "val1", "arg2" => "val2" },
    environ => { "env1" => "val1", "env2" => "val2" },
    headers => { "head1" => "val1", "head2" => "val2" },
    operation => "get",
    component => "component",
    version => "v1",
    job_type => "sync",
    remote => { ip => "1.2.3.4", "user" => "user" },
    resource => { type => "resource", name => $id },
    base_uri => "http://localhost:4080/component/v1",
    parent_uri => "http://localhost:4080/component/v1/status/s-12345",
    on => {
        "COMPLETED" => {
            "name" => "do_get_component_other_v1",
            "envelope" => {
                arguments => [],
                matrix_arguments => {},
                query_params => {},
                environ => {},
                headers => {},
                operation => "get",
                component => "component",
                version => "v1",
                job_type => "async",
                resource => { type => "other" },
                base_uri => "http://localhost:4080/component/v1",
                parent_uri => "http://localhost:4080/component/v1/status/s-12345",
            }
        }
    }
};

OK my $resp = $job->run();

# test copy ctor
OK $copy = Gearbox::JobResponse->new($resp);
IS $copy->content() => $resp->content();
IS $copy->headers() => $resp->headers();
IS $copy->status()->serialize() => $resp->status()->serialize();
IS $copy->code() => $resp->code();
IS $copy->job()->serialize() => $resp->job()->serialize();

undef $copy;

IS $resp->content() => '{"key":"value"}';
IS $resp->headers() => {};
my $status;
NOTHROW { $status = from_json($resp->status()->serialize()) };
LIKE delete $status->{ctime} => qr{\d{10}};
LIKE delete $status->{mtime} => qr{\d{10}};
LIKE delete $status->{status_uri} => qr{^http://localhost:4080/transient/status/st-[a-z0-9]+$};
IS $status => {
    failures => 0,
    messages => [],
    progress => 100,
    children => [],
    uri => "http://localhost:4080/component/v1/resource/$id",
    state => "COMPLETED",
    component => "component",
    operation => "get",
    code => 0,
    concurrency => 0
};
IS from_json($resp->job()->serialize()) => {
    content => '{"this":"that"}',
    arguments => ["arg1", "arg2"],
    matrix_arguments => {"arg1" => "val1", "arg2" => "val2"},
    query_params => { "arg1" => "val1", "arg2" => "val2" },
    environ => { "env1" => "val1", "env2" => "val2" },
    headers => { "head1" => "val1", "head2" => "val2" },
    operation => "get",
    component => "component",
    version => "v1",
    job_type => "sync",
    remote => { ip => "1.2.3.4", "user" => "user" },
    resource => { type => "resource", name => $id },
    base_uri => "http://localhost:4080/component/v1",
    parent_uri => "http://localhost:4080/component/v1/status/s-12345",
    on => {
        "COMPLETED" => {
            "name" => "do_get_component_other_v1",
            "envelope" => {
                arguments => [],
                matrix_arguments => {},
                query_params => {},
                environ => {},
                headers => {},
                operation => "get",
                component => "component",
                version => "v1",
                job_type => "async",
                resource => { type => "other" },
                base_uri => "http://localhost:4080/component/v1",
                parent_uri => "http://localhost:4080/component/v1/status/s-12345",
            }
        }
    }
};

IS $resp->code() => 200;



# call resetters
NOTHROW { $job->arguments([]) };
IS $job->arguments() => [];

NOTHROW { $job->matrix_arguments({}) };
IS $job->matrix_arguments() => {};

NOTHROW { $job->query_params({}) };
IS $job->query_params() => {};

NOTHROW { $job->headers({}) };
IS $job->headers() => {};

NOTHROW { $job->environ({}) };
IS $job->environ() => {};
