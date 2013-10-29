#!/usr/bin/env python

import setup
import json

from testtrivial import *
from Gearbox import ConfigFile, Job, JobResponse, JobManager, HttpClient

PLAN(106)

cfg = ConfigFile("./unit.conf")

IS( Job.JOB_UNKNOWN, 0 )
IS( Job.JOB_SYNC, 1 )
IS( Job.JOB_ASYNC, 2 )

IS( Job.event2str( Job.EVENT_UNKNOWN ), "UNKNOWN" )
IS( Job.event2str( Job.EVENT_COMPLETED ), "COMPLETED" )
IS( Job.event2str( Job.EVENT_FAILED ), "FAILED" )
IS( Job.event2str( Job.EVENT_SUCCEEDED ), "SUCCEEDED" )
IS( Job.event2str( Job.EVENT_STARTED ), "STARTED" )
IS( Job.event2str( Job.EVENT_STOPPED ), "STOPPED" )
IS( Job.event2str( Job.EVENT_CANCELLED ), "CANCELLED" )

IS( Job.str2event("UNKNOWN"), Job.EVENT_UNKNOWN )
IS( Job.str2event("COMPLETED"), Job.EVENT_COMPLETED )
IS( Job.str2event("FAILED"), Job.EVENT_FAILED )
IS( Job.str2event("SUCCEEDED"), Job.EVENT_SUCCEEDED )
IS( Job.str2event("STARTED"), Job.EVENT_STARTED )
IS( Job.str2event("STOPPED"), Job.EVENT_STOPPED )
IS( Job.str2event("CANCELLED"), Job.EVENT_CANCELLED )


jm = (OK( JobManager(cfg) ))[1]
jm.base_uri("http://localhost:4080/component/v1")
jm.parent_uri("http://localhost:4080/component/v1/status/s-12345")

NOK( jm.known_job_name("do_foobar") )
OK( jm.known_job_name("do_get_component_resource_v1") )
OK( jm.known_job_name("do_get_component_other_v1") )

# test jobs without schemas
jm.requireSchemas(1);
job = (NOTHROW( lambda: jm.job("do_get_component_other_v1") ))[1]
IS( str(THROWS( lambda: job.run()) ), 'Required schema for job do_get_component_other_v1 not found!')

jm.requireSchemas(0);
OK( jm.job("do_get_component_other_v1") )

# test jobs with schemas
jm.requireSchemas(1);
job = (OK( jm.job("do_get_component_resource_v1") ))[1]
NOTHROW( lambda: job.content('{"key": "value"}') )
IS( str(THROWS( lambda: job.run()) ), 'Json Exception: invalid property "key": schema does not allow for this property' )

# this should work with the schema
job.content('{"this":"that"}')
IS( job.json_content(), { "this": "that" } )

# turn of schema requirements for futher testing
jm.requireSchemas(0);
OK( jm.job(HttpClient.METHOD_GET, "http://localhost:4080/component/v1/resource") )

# test copy ctor
copy = (OK( Job(job) ))[1]
IS( job.serialize(), copy.serialize() )
# delete job, verify that copy is still good
envelope = job.serialize()
job = None
IS( copy.serialize(), envelope )

# test delay/retry
NOTHROW( lambda: jm.delay(copy, 10) )
NOTHROW( lambda: jm.retry(copy) )

# test gen_id
LIKE( jm.gen_id('r'), '^r-[a-z0-9]+$' )

# test all job accessors:
job = (OK( jm.job("do_get_component_resource_v1") ))[1]
NOTHROW( lambda: job.content('{"this":"that"}') )
IS( job.json_content(), { "this": "that" } )

NOTHROW( lambda: job.add_argument("arg1") )
NOTHROW( lambda: job.add_argument("arg2") )
IS( job.arguments(), ("arg1", "arg2") )

NOTHROW( lambda: job.add_matrix_argument("arg1", "val1") )
NOTHROW( lambda: job.add_matrix_argument("arg2", "val2") )
IS( job.matrix_arguments(), {'arg1': "val1", 'arg2': "val2"} )

NOTHROW( lambda: job.add_query_param("arg1", "val1") )
NOTHROW( lambda: job.add_query_param("arg2", "val2") )
IS( job.query_params(), {'arg1': "val1", 'arg2': "val2"} )

NOTHROW( lambda: job.add_header("head1", "val1") )
NOTHROW( lambda: job.add_header("head2", "val2") )
IS( job.headers(), {'head1': "val1", 'head2': "val2"} )

NOTHROW( lambda: job.add_environ("env1", "val1") )
NOTHROW( lambda: job.add_environ("env2", "val2") )
IS( job.environ(), {'env1': "val1", 'env2': "val2"} )

IS( job.status(), "" )
IS( job.base_uri(), "http://localhost:4080/component/v1" )
IS( job.name(), "do_get_component_resource_v1" )
IS( job.api_version(), "v1" )
IS( job.operation(), "get" )
IS( job.component(), "component" )
IS( job.resource_type(), "resource" )

id = jm.gen_id('r');
NOTHROW( lambda: job.resource_name( id ) )
IS( job.resource_name(), id )

IS( job.type(), Job.JOB_ASYNC )
NOTHROW( lambda: job.type(Job.JOB_SYNC) )
IS( job.type(), Job.JOB_SYNC )

IS( job.resource_uri(), "http://localhost:4080/component/v1/resource/" + id )

NOTHROW( lambda: job.remote_ip("1.2.3.4") )
IS( job.remote_ip(), "1.2.3.4" )

NOTHROW( lambda: job.remote_user("user") )
IS( job.remote_user(), "user" )

# test on(EVENT) apis
IS( job.on(Job.EVENT_COMPLETED), None )

other = jm.job("do_get_component_other_v1");
NOTHROW( lambda: job.on(Job.EVENT_COMPLETED, other) )
IS( json.loads( job.on(Job.EVENT_COMPLETED).serialize() ), {
    "arguments": [],
    "matrix_arguments": {},
    "query_params": {},
    "environ": {},
    "headers": {},
    "operation": "get",
    "component": "component",
    "version": "v1",
    "job_type": "async",
    "resource": { 'type': "other" },
    "base_uri": "http://localhost:4080/component/v1",
    "parent_uri": "http://localhost:4080/component/v1/status/s-12345",
} )

IS( json.loads( job.serialize() ), {
    "content": '{"this":"that"}',
    "arguments": ["arg1", "arg2"],
    "matrix_arguments": {"arg1": "val1", "arg2": "val2"},
    "query_params": { "arg1": "val1", "arg2": "val2" },
    "environ": { "env1": "val1", "env2": "val2" },
    "headers": { "head1": "val1", "head2": "val2" },
    "operation": "get",
    "component": "component",
    "version": "v1",
    "job_type": "sync",
    "remote": { 'ip': "1.2.3.4", "user": "user" },
    "resource": { "type": "resource", "name": id },
    "base_uri": "http://localhost:4080/component/v1",
    "parent_uri": "http://localhost:4080/component/v1/status/s-12345",
    "on": {
        "COMPLETED": {
            "name": "do_get_component_other_v1",
            "envelope": {
                "arguments": [],
                "matrix_arguments": {},
                "query_params": {},
                "environ": {},
                "headers": {},
                "operation": "get",
                "component": "component",
                "version": "v1",
                "job_type": "async",
                "resource": { 'type': "other" },
                "base_uri": "http://localhost:4080/component/v1",
                "parent_uri": "http://localhost:4080/component/v1/status/s-12345",
            }
        }
    }
} )

resp = (OK( job.run() ))[1]

# test copy ctor
copy = (OK( JobResponse(resp) ))[1]
IS( copy.content(), resp.content() )
IS( copy.headers(), resp.headers() )
IS( copy.status().serialize(), resp.status().serialize() )
IS( copy.code(), resp.code() )
IS( copy.job().serialize(), resp.job().serialize() )

# destroying copy shouldn't change original
copy = None
IS( resp.content(), '{"key":"value"}' )
IS( resp.headers(), {} )


status = (NOTHROW( lambda: json.loads(resp.status().serialize()) ))[1]
ctime = status["ctime"]
del status["ctime"]
OK( isinstance( ctime, int ) )

mtime = status["mtime"]
del status["mtime"]
OK( isinstance( mtime, int ) )

status_uri = status["status_uri"]
del status["status_uri"]
LIKE( status_uri, '^http://localhost:4080/transient/status/st-[a-z0-9]+$' )

IS( status, {
    u"failures": 0,
    u"messages": [],
    u"progress": 100,
    u"children": [],
    u"uri": u"http://localhost:4080/component/v1/resource/"+id,
    u"state": u"COMPLETED",
    u"component": u"component",
    u"operation": u"get",
    u"code": 0,
    u"concurrency": 0
})

IS( json.loads(resp.job().serialize()), {
    'content': '{"this":"that"}',
    'arguments': ["arg1", "arg2"],
    'matrix_arguments': {"arg1": "val1", "arg2": "val2"},
    'query_params': { "arg1": "val1", "arg2": "val2" },
    'environ': { "env1": "val1", "env2": "val2" },
    'headers': { "head1": "val1", "head2": "val2" },
    'operation': "get",
    'component': "component",
    'version': "v1",
    'job_type': "sync",
    'remote': { "ip": "1.2.3.4", "user": "user" },
    'resource': { "type": "resource", 'name': id },
    'base_uri': "http://localhost:4080/component/v1",
    'parent_uri': "http://localhost:4080/component/v1/status/s-12345",
    'on': {
        "COMPLETED": {
            "name": "do_get_component_other_v1",
            "envelope": {
                'arguments': [],
                'matrix_arguments': {},
                'query_params': {},
                'environ': {},
                'headers': {},
                'operation': "get",
                'component': "component",
                'version': "v1",
                'job_type': "async",
                'resource': { 'type': "other" },
                'base_uri': "http://localhost:4080/component/v1",
                'parent_uri': "http://localhost:4080/component/v1/status/s-12345",
            }
        }
    }
})

IS( resp.code(), 200 )

# test passing a hash that contains something other than a string
IS( str(THROWS( lambda: job.matrix_arguments({ "foo": 123}) )), "Dictionary values can only be strings." )
IS( str(THROWS( lambda: job.matrix_arguments({ 123: "foo"}) )), "Dictionary keys can only be strings." )

# call resetters
NOTHROW( lambda: job.arguments([]) )
IS( job.arguments(), () )

job.matrix_arguments({})
NOTHROW( lambda: job.matrix_arguments({}) )
IS( job.matrix_arguments(), {} )

NOTHROW( lambda: job.query_params({}) )
IS( job.query_params(), {} )

NOTHROW( lambda: job.headers({}) )
IS( job.headers(), {} )

NOTHROW( lambda: job.environ({}) )
IS( job.environ(), {} )
