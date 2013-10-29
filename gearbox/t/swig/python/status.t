#!/usr/bin/env python

import setup
import json
import time

from gearbox import Status, StatusManager, ConfigFile, JobManager
from testtrivial import *

PLAN(81)

IS( Status.state2str(Status.STATE_UNKNOWN), "UNKNOWN" )
IS( Status.state2str(Status.STATE_PENDING), "PENDING" )
IS( Status.state2str(Status.STATE_RUNNING), "RUNNING" )
IS( Status.state2str(Status.STATE_STOPPING), "STOPPING" )
IS( Status.state2str(Status.STATE_STOPPED), "STOPPED" )
IS( Status.state2str(Status.STATE_CANCELLING), "CANCELLING" )
IS( Status.state2str(Status.STATE_COMPLETED), "COMPLETED" )

IS( Status.str2state("UNKNOWN"), Status.STATE_UNKNOWN )
IS( Status.str2state("PENDING"), Status.STATE_PENDING )
IS( Status.str2state("RUNNING"), Status.STATE_RUNNING )
IS( Status.str2state("STOPPING"), Status.STATE_STOPPING )
IS( Status.str2state("STOPPED"), Status.STATE_STOPPED )
IS( Status.str2state("CANCELLING"), Status.STATE_CANCELLING )
IS( Status.str2state("COMPLETED"), Status.STATE_COMPLETED )

IS( Status.event2str(Status.EVENT_UNKNOWN), "UNKNOWN" )
IS( Status.event2str(Status.EVENT_PRECANCEL), "PRECANCEL" )
IS( Status.event2str(Status.EVENT_CANCEL), "CANCEL" )

IS( Status.str2event("UNKNOWN"), Status.EVENT_UNKNOWN )
IS( Status.str2event("PRECANCEL"), Status.EVENT_PRECANCEL )
IS( Status.str2event("CANCEL"), Status.EVENT_CANCEL )

sm = (OK( StatusManager(ConfigFile("./unit.conf")) ))[1]
BASE = "http://localhost:4080/transient/"
NOTHROW( lambda: sm.base_uri(BASE) )

p = (OK( sm.create("s-0", "operation", BASE + "/thing", "component") ))[1]
s = (OK( sm.create("s-1", "operation", BASE + "/thing", "component") ))[1]
IS( s.name(), "s-1" )
IS( s.operation(), "operation" )
IS( s.resource_uri(), BASE + "/thing" )
IS( s.component(), "component" )
IS( s.uri(), "http://localhost:4080/transient/status/s-1" )

IS( sm.fetch("s-1").serialize(), s.serialize() )
IS( sm.fetch(json.loads(s.serialize())).serialize(), s.serialize() )

# setup up parent
NOTHROW( lambda: s.parent_uri(p.uri()) )
IS( s.parent_uri(), p.uri() )
IS( s.parent().serialize(), p.serialize() )

# test message apis
IS( s.messages(), () )
NOTHROW( lambda: s.add_message("message") )
IS( s.messages(), ("message",) )

# test children apis
IS( s.children(), () )
NOTHROW( lambda: s.add_child(BASE + "/status/s-2") )
IS( s.children(), (BASE + "/status/s-2",) )

# test meta apis
IS( s.meta(), None )
NOTHROW( lambda: s.meta("key", "value") )
IS( s.meta(), { 'key': "value" } )
NOTHROW( lambda: s.meta( { 'a': 1, 'b': 2} ) )
IS( s.meta(), {'a': 1, 'b': 2} )

# test progress apis
IS( s.progress(), 0 )
NOTHROW( lambda: s.progress(50) )
IS( s.progress(), 50 )

# fail the status, set progress, code and state
NOTHROW( lambda: s.fail(123) )
IS( s.progress(), 100 )
IS( s.code(), 123 )
IS( s.state(), Status.STATE_COMPLETED )

# set to successful, progress, code and state updated
NOTHROW( lambda: s.success() )
IS( s.progress(), 100 )
IS( s.code(), 0 )
IS( s.state(), Status.STATE_COMPLETED )

NOTHROW( lambda: s.checkpoint() )

# test cancel api
NOTHROW( lambda: s.state(Status.STATE_CANCELLING) )
NOTHROW( lambda: s.cancel() )
IS( s.code(), 0 )
IS( s.state(), Status.STATE_CANCELLED )

IS( str(THROWS( lambda: s.checkpoint() )), "Worker Stop" )

jm = (OK( JobManager(ConfigFile("./unit.conf")) ))[1]
job = (OK( jm.job("do_get_component_other_v1") ))[1]

NOTHROW( lambda: s.on(Status.EVENT_PRECANCEL, job) )
IS( s.on(Status.EVENT_PRECANCEL, jm).name(), job.name() )

NOTHROW( lambda: s.on(Status.EVENT_CANCEL, job) )
IS( s.on(Status.EVENT_CANCEL, jm).name(), job.name() )

OK( s.ctime() <= time.time() )
OK( s.mtime() <= time.time() )

IS( s.ytime(), -1 )
NOTHROW( lambda: s.ytime(0) )
IS( s.ytime(), 0 )

NOTHROW( lambda: s.sync() )

IS( s.has_completed(), 1 )
OK( s.is_success() )

IS( s.failures(), 0 )
NOTHROW( lambda: s.failures(1) )
IS( s.failures(), 1 )


IS( json.loads(s.serialize()), {
    'failures': 1,
    'ctime': s.ctime(),
    'messages': [
        'message'
    ],
    'progress': 100,
    'mtime': s.mtime(),
    'children': [
        'http://localhost:4080/transient/status/s-2'
    ],
    'uri': 'http://localhost:4080/transient//thing',
    'state': 'CANCELLED',
    'meta': {
        'a': 1,
        'b': 2
    },
    'parent_uri': 'http://localhost:4080/transient/status/s-0',
    'component': 'component',
    'status_uri': 'http://localhost:4080/transient/status/s-1',
    'operation': 'operation',
    'code': 0,
    'concurrency': 0
} )
