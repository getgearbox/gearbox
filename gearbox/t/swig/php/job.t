#!/usr/bin/env phprun -s # -*- mode: php-*-
<?
require_once("Gearbox.php");
require_once("test-trivial.php");

PLAN(101);

OK( $cfg = new GearboxConfigFile("./unit.conf") );

Gearbox::log_init("./unit.conf");

# test job type constants
IS( GearboxJob::JOB_UNKNOWN, 0 );
IS( GearboxJob::JOB_SYNC, 1 );
IS( GearboxJob::JOB_ASYNC, 2 );

# type job event constants
IS( GearboxJob::event2str(GearboxJob::EVENT_UNKNOWN), "UNKNOWN" );
IS( GearboxJob::event2str(GearboxJob::EVENT_COMPLETED), "COMPLETED" );
IS( GearboxJob::event2str(GearboxJob::EVENT_FAILED), "FAILED" );
IS( GearboxJob::event2str(GearboxJob::EVENT_SUCCEEDED), "SUCCEEDED" );
IS( GearboxJob::event2str(GearboxJob::EVENT_STARTED), "STARTED" );
IS( GearboxJob::event2str(GearboxJob::EVENT_STOPPED), "STOPPED" );
IS( GearboxJob::event2str(GearboxJob::EVENT_CANCELLED), "CANCELLED" );

IS( GearboxJob::str2event("UNKNOWN"), GearboxJob::EVENT_UNKNOWN );
IS( GearboxJob::str2event("COMPLETED"), GearboxJob::EVENT_COMPLETED );
IS( GearboxJob::str2event("FAILED"), GearboxJob::EVENT_FAILED );
IS( GearboxJob::str2event("SUCCEEDED"), GearboxJob::EVENT_SUCCEEDED );
IS( GearboxJob::str2event("STARTED"), GearboxJob::EVENT_STARTED );
IS( GearboxJob::str2event("STOPPED"), GearboxJob::EVENT_STOPPED );
IS( GearboxJob::str2event("CANCELLED"), GearboxJob::EVENT_CANCELLED );

OK( $jm = new GearboxJobManager($cfg) );
$jm->base_uri("http://localhost:4080/component/v1");
$jm->parent_uri("http://localhost:4080/component/v1/status/s-12345");

NOK( $jm->known_job_name("do_foobar") );
OK( $jm->known_job_name("do_get_component_resource_v1") );
OK( $jm->known_job_name("do_get_component_other_v1") );

# test jobs without schemas
$jm->requireSchemas(1);
$job = NOTHROW( function() use($jm) { return $jm->job("do_get_component_other_v1"); } );
IS( THROWS( function() use($job) { $job->run(); } ), 'Required schema for job do_get_component_other_v1 not found!' );

$jm->requireSchemas(0);
OK( $job = $jm->job("do_get_component_other_v1") );

# test jobs with schemas
$jm->requireSchemas(1);
OK( $job = $jm->job("do_get_component_resource_v1") );
NOTHROW( function() use($job) { $job->content('{"key": "value"}'); } );
IS( THROWS ( function() use($job) { $job->run(); } ), 'Json Exception: invalid property "key": schema does not allow for this property' );

# this should work with the schema
$job->content('{"this":"that"}');
IS( $job->json_content(), array("this" => "that" ) );

# turn of schema requirements for futher testing
$jm->requireSchemas(0);

OK( $job = $jm->job(GearboxHttpClient::METHOD_GET, "http://localhost:4080/component/v1/resource") );

# test copy ctor
OK( $copy = new GearboxJob($job) );
IS( $job->serialize(), $copy->serialize() );

# delete job, verify that copy is still good
$envelope = $job->serialize();
$job = null;
IS( $copy->serialize(), $envelope );

# test delay/retry
NOTHROW( function() use($jm, $copy) { $jm->delay($copy, 10); } );
NOTHROW( function() use($jm, $copy) { $jm->retry($copy); } );

# # test gen_id
LIKE( $jm->gen_id('r'), "/^r-[a-z0-9]+$/" );

# test all job accessors:
OK( $job = $jm->job("do_get_component_resource_v1") );
NOTHROW( function() use($job) { $job->content('{"this":"that"}'); } );
IS( $job->json_content(), array( "this" => "that" ) );

NOTHROW( function() use($job) { $job->add_argument("arg1"); } );
NOTHROW( function() use($job) { $job->add_argument("arg2"); } );
IS( $job->arguments(), array("arg1", "arg2") );

NOTHROW( function() use($job) { $job->add_matrix_argument("arg1", "val1"); } );
NOTHROW( function() use($job) { $job->add_matrix_argument("arg2", "val2"); } );
IS( $job->matrix_arguments(), array('arg1' => "val1", 'arg2' => "val2") );

NOTHROW( function() use($job) { $job->add_query_param("arg1", "val1"); } );
NOTHROW( function() use($job) { $job->add_query_param("arg2", "val2"); } );
IS( $job->query_params(), array( 'arg1' => "val1", 'arg2' => "val2") );

NOTHROW( function() use($job) { $job->add_header("head1", "val1"); } );
NOTHROW( function() use($job) { $job->add_header("head2", "val2"); } );
IS( $job->headers(), array( 'head1' => "val1", 'head2' => "val2") );

NOTHROW( function() use($job) { $job->add_environ("env1", "val1"); } );
NOTHROW( function() use($job) { $job->add_environ("env2", "val2"); } );
IS( $job->environ(), array( 'env1' => "val1", 'env2' => "val2") );

IS( $job->status(), "" );
IS( $job->base_uri(), "http://localhost:4080/component/v1" );
IS( $job->name(), "do_get_component_resource_v1" );
IS( $job->api_version(), "v1" );
IS( $job->operation(), "get" );
IS( $job->component(), "component" );
IS( $job->resource_type(), "resource" );

$id = $jm->gen_id('r');
NOTHROW( function() use($job, $id) { $job->resource_name( $id ); } );
IS( $job->resource_name(), $id );

IS( $job->type(), GearboxJob::JOB_ASYNC );
NOTHROW( function() use($job) { $job->type(GearboxJob::JOB_SYNC); } );
IS( $job->type(), GearboxJob::JOB_SYNC );

IS( $job->resource_uri(), "http://localhost:4080/component/v1/resource/$id" );

NOTHROW( function() use($job) { $job->remote_ip("1.2.3.4"); } );
IS( $job->remote_ip(), "1.2.3.4" );

NOTHROW( function() use($job) { $job->remote_user("user"); } );
IS( $job->remote_user(), "user" );

# test on(EVENT) apis
IS( $job->on(GearboxJob::EVENT_COMPLETED), null );

$other = $jm->job("do_get_component_other_v1");
NOTHROW( function() use($job, $other) { $job->on(GearboxJob::EVENT_COMPLETED, $other); } );
IS( json_decode( $job->on(GearboxJob::EVENT_COMPLETED)->serialize(), true ), 
    array(
        'arguments'=> array(),
        'base_uri' => "http://localhost:4080/component/v1",
        'component' => "component",
        'environ' => array(),
        'headers' => array(),
        'job_type' => "async",
        'matrix_arguments' => array(),
        'operation' => "get",
        'parent_uri' => "http://localhost:4080/component/v1/status/s-12345",
        'query_params' => array(),
        'resource' => array( 'type' => "other" ),
        'version' => "v1"
    )
);

IS( json_decode( $job->serialize(), true),
    array(
        'content' => '{"this":"that"}',
        'arguments' => array("arg1", "arg2"),
        'matrix_arguments' => array("arg1" => "val1", "arg2" => "val2"),
        'query_params' => array( "arg1" => "val1", "arg2" => "val2" ),
        'environ' => array( "env1" => "val1", "env2" => "val2" ),
        'headers' => array( "head1" => "val1", "head2" => "val2" ),
        'operation' => "get",
        'component' => "component",
        'version' => "v1",
        'job_type' => "sync",
        'remote' => array( 'ip' => "1.2.3.4", "user" => "user" ),
        'resource' => array( 'type' => "resource", 'name' => $id ),
        'base_uri' => "http://localhost:4080/component/v1",
        'parent_uri' => "http://localhost:4080/component/v1/status/s-12345",
        'on' => array(
            "COMPLETED" => array(
                "name" => "do_get_component_other_v1",
                "envelope" => array(
                    'arguments' => array(),
                    'matrix_arguments' => array(),
                    'query_params' => array(),
                    'environ' => array(),
                    'headers' => array(),
                    'operation' => "get",
                    'component' => "component",
                    'version' => "v1",
                    'job_type' => "async",
                    'resource' => array( 'type' => "other" ),
                    'base_uri' => "http://localhost:4080/component/v1",
                    'parent_uri' => "http://localhost:4080/component/v1/status/s-12345",
                )
            )
        )
    )
);

OK( $resp = $job->run() );

# test copy ctor
OK( $copy = new GearboxJobResponse($resp) );
IS( $copy->content(), $resp->content() );
IS( $copy->headers(), $resp->headers() );
IS( $copy->status()->serialize(), $resp->status()->serialize() );
IS( $copy->code(), $resp->code() );
IS( $copy->job()->serialize(), $resp->job()->serialize() );

unset($copy);

IS( $resp->content(), '{"key":"value"}' );
IS( $resp->headers(), array() );

$status = NOTHROW( function() use($resp) { return json_decode($resp->status()->serialize(), true); } );
LIKE( $status["ctime"], "/\d{10}/" );
unset($status["ctime"]);
LIKE( $status["mtime"], "/\d{10}/" );
unset($status["mtime"]);
LIKE( $status["status_uri"], "!^http://localhost:4080/transient/status/st-[a-z0-9]+$!" );
unset($status["status_uri"]);
IS( $status, array(
    children => array(),
    code => 0,
    component => "component",
    concurrency => 0,
    failures => 0,
    messages => array(),
    operation => "get",
    progress => 100,
    state => "COMPLETED",
    uri => "http://localhost:4080/component/v1/resource/$id"
) );
IS( json_decode($resp->job()->serialize(),true),  array(
    'arguments' => array("arg1", "arg2"),
    'base_uri' => "http://localhost:4080/component/v1",
    'component' => "component",
    'content' => '{"this":"that"}',
    'environ' => array( "env1" => "val1", "env2" => "val2" ),
    'headers' => array( "head1" => "val1", "head2" => "val2" ),
    'job_type' => "sync",
    'matrix_arguments' => array("arg1" => "val1", "arg2" => "val2"),
    'on' => array(
        "COMPLETED" => array(
            "envelope" => array(
                'arguments' => array(),
                'base_uri' => "http://localhost:4080/component/v1",
                'component' => "component",
                'environ' => array(),
                'headers' => array(),
                'job_type' => "async",
                'matrix_arguments' => array(),
                'operation' => "get",
                'parent_uri' => "http://localhost:4080/component/v1/status/s-12345",
                'query_params' => array(),
                'resource' => array( 'type' => "other" ),
                'version' => "v1"
            ),
            "name" => "do_get_component_other_v1"
        )
    ),
    'operation' => "get",
    'parent_uri' => "http://localhost:4080/component/v1/status/s-12345",
    'query_params' => array( "arg1" => "val1", "arg2" => "val2" ),
    'remote' => array( 'ip' => "1.2.3.4", "user" => "user" ),
    'resource' => array( 'type' => "resource", 'name' => $id ),
    'version' => "v1"
) );

IS( $resp->code(), 200 );



# call resetters
NOTHROW( function() use($job) { $job->arguments(array()); } );
IS( $job->arguments(), array() );

NOTHROW( function() use($job) { $job->matrix_arguments(array()); } );
IS( $job->matrix_arguments(), array() );

NOTHROW( function() use($job) { $job->query_params(array()); } );
IS( $job->query_params(), array() );

NOTHROW( function() use($job) { $job->headers(array()); } );
IS( $job->headers(), array() );

NOTHROW( function() use($job) { $job->environ(array()); } );
IS( $job->environ(), array() );
?>
