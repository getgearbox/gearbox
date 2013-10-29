#!/usr/bin/env python

import setup

from testtrivial import *
from gearbox import ConfigFile

PLAN(17)

vars = Vars()
OK( vars.set("cfg", ConfigFile("./unit.conf") ) )
cfg = vars["cfg"]
IS( cfg.get_string("test-string"), "string" )
IS( cfg.get_string("test-object", "test-string"), "string" )
IS( cfg.get_string_default("does-not-exist", "default"), "default" )
IS( cfg.get_string_default("does-not-exist", "does-not-exist", "default"), "default" )
IS( cfg.get_string_default("test-object", "does-not-exist", "default"), "default" )
IS( str(THROWS( lambda: cfg.get_string_default("test-int", "default") )), 'Json Exception: cannot convert integer to string at: ["test-int"]' )

IS( cfg.get_int("test-int"), 1 )
IS( cfg.get_int("test-object", "test-int"), 1 )
IS( cfg.get_int_default("does-not-exist", 42), 42 )
IS( cfg.get_int_default("does-not-exist", "does-not-exist", 42), 42 )
IS( cfg.get_int_default("test-object", "does-not-exist", 42), 42 )
IS( str(THROWS( lambda: cfg.get_int_default("test-string", 42) )), 'Json Exception: cannot convert string to integer at: ["test-string"]' )

IS( cfg.get_json("test-object"), {
    u"test-int": 1,
    u"test-double": 1.2,
    u"test-string": u"string",
    u"test-bool": True
} )

IS( cfg.as_json(), {
    u"component": u"unit",
    u"db_type": u"sqlite3",
    u"db_name": u"./.db/test.db",
    u"log": {
        u"config_file": u"../../../../common/conf/stdout-logger.conf"
    },
    u"schemadir": u"./schemas",
    u"status": {
        u"persistence_type": u"transient"
    },
    u"gearbox": {
        u"conf": u"."
    },
    u"scoreboard": {
        u"name": u"./unit"
    },
    u"test-int": 1,
    u"test-double": 1.2,
    u"test-string": u"string",
    u"test-bool": True,
    u"test-object": {
        u"test-int": 1,
        u"test-double": 1.2,
        u"test-string": u"string",
        u"test-bool": True
    },
    u"test-array": [
       u"test"
    ]
} )
