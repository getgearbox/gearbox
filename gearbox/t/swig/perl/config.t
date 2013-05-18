#!/usr/bin/env perl
use lib $ENV{PWD}."/".($0 =~ m{(.*)/[^/]+$})[0];
use strict;
use warnings;
use setup;
use Test::Trivial (tests => 15);

use Gearbox::Logger;
Gearbox::Logger->init("./unit.conf");

use Gearbox;
OK my $cfg = Gearbox::ConfigFile->new("./unit.conf");

IS $cfg->get_string("test-string") => "string";
IS $cfg->get_string("test-object", "test-string") => "string";
IS $cfg->get_string_default("does-not-exist", "default") => "default";
IS $cfg->get_string_default("does-not-exist", "does-not-exist", "default") => "default";
IS $cfg->get_string_default("test-object", "does-not-exist", "default") => "default";
IS THROWS { $cfg->get_string_default("test-int", "default") }
    => qq{Json Exception: cannot convert integer to string at: ["test-int"]};

IS $cfg->get_int("test-int") => 1;
IS $cfg->get_int("test-object", "test-int") => 1;
IS $cfg->get_int_default("does-not-exist", 42) => 42;
IS $cfg->get_int_default("does-not-exist", "does-not-exist", 42) => 42;
IS $cfg->get_int_default("test-object", "does-not-exist", 42) => 42;
IS THROWS { $cfg->get_int_default("test-string", 42) }
    => qq{Json Exception: cannot convert string to integer at: ["test-string"]};

IS $cfg->get_json("test-object") => {
    "test-int" => 1,
    "test-double"  => 1.2,
    "test-string" => "string",
    "test-bool" => "true"
};

IS $cfg->as_json() => {
    "component" => "unit",
    "db_type"  => "sqlite3",
    "db_name"  => "./.db/test.db",
    "log"  => {
        "config_file"  => "../../../../common/conf/stdout-logger.conf"
    },
    "schemadir" => "./schemas",
    "status" => {
        "persistence_type" => "transient"
    },
    "gearbox" => {
        "conf" => "."
    },
    "scoreboard" => {
        "name" => "./unit"
    },
    "test-int" => 1,
    "test-double"  => 1.2,
    "test-string" => "string",
    "test-bool" => "true",
    "test-object" => {
        "test-int" => 1,
        "test-double"  => 1.2,
        "test-string" => "string",
        "test-bool" => "true"
    },
    "test-array" => [
       "test"
    ]
};
