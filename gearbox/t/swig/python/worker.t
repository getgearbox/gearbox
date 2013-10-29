#!/usr/bin/env python

import setup

from gearbox import Worker
from testtrivial import *

class TestWorker(Worker):
    def __init__(self, config):
        # this is the order we will run the jobs so that we can test the names
        # in the pre/post request handlers
        self.queue = "do_run_handler_thing_v1 do_run_handler_thing_v2".split();

        super(TestWorker, self).__init__(config)

    def do_run_handler_thing_v2(self, job, resp):
        IS( job.api_version(), 'v2' )
        return Worker.WORKER_SUCCESS;

    # these get called automatically when a job comes in, verify the job name
    # is correct and verify no exceptions from parent pre/post request handlers
    def pre_request(self, job):
        IS( job.name(), self.queue[0] )
        NOTHROW( lambda: super(TestWorker, self).pre_request(job) )

    def post_request(self, job):
        IS( job.name(), self.queue.pop(0) )
        NOTHROW( lambda: super(TestWorker, self).post_request(job) )

PLAN(39)

IS( str(THROWS( lambda: Worker("bogusfile.conf") )), "config file 'bogusfile.conf' is not a file" )

worker = (OK( TestWorker("./unit.conf") ))[1]
IS( str(THROWS( lambda: worker.set_self("foo") )), "set_self argument must be an instance of Worker" )

jm = (OK( worker.job_manager() ))[1]
NOTHROW( lambda: jm.base_uri("http://localhost:4080/test/v1") )

sm = (OK( worker.job_manager() ))[1]
NOTHROW( lambda: sm.base_uri("http://localhost:4080/test/v1") )

# test constants
IS( Worker.WORKER_SUCCESS, 0 )
IS( Worker.WORKER_ERROR, 1 )
IS( Worker.WORKER_CONTINUE, 2 )
IS( Worker.WORKER_RETRY, 3 )

# verify 2 forms of register_handler
# work.  Each handler will check api_version of
# incoming job to make sure it matches our api

# other language bindings allow you to pass an anonymous function to
# register_handler.  Python doesn't have anonymous functions, so this
# form isn't really possible.  It does have lambda, but lambda can
# only contain a single expression, which isn't very useful for writing
# workers.

def handler(self, job, resp):
    IS( job.api_version(), 'v1' )
    return Worker.WORKER_SUCCESS;

NOTHROW( lambda: worker.register_handler( "do_run_handler_thing_v1",  handler ) )
NOTHROW( lambda: worker.register_handler( "do_run_handler_thing_v2") )

IS( str(THROWS( lambda: worker.register_handler( "bogus") )), "bogus does not exist in class")

# reset max_requests so that we can test the worker::run
# (otherwise it is an infinite loop)

NOTHROW( lambda: worker.max_requests(1) )
IS( worker.request_count(), 0 )

# create v1 job, add it to queue, then let worker run it
job = (OK( jm.job("do_run_handler_thing_v1") ))[1]
NOTHROW( lambda: job.run() )
NOTHROW( lambda: worker.run() )

# update to v2 job, add it to queue, then let worker run it
NOTHROW( lambda: job.name("do_run_handler_thing_v2").run() )
NOTHROW( lambda: worker.run() )

# we have handled requests, count should be updated
IS( worker.request_count(), 2 )

# verify aftewards jobs dont throw exceptions
NOTHROW( lambda: worker.afterwards( job ) )
NOTHROW( lambda: worker.afterwards( job, 30 ) )

cfg = (OK(worker.cfg()))[1]
IS( cfg.get_string("component"), "unit" )
