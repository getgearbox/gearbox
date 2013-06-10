#!/usr/bin/env phprun
<?

require_once("Gearbox.php");
require_once("test-trivial.php");

PLAN(15);

Gearbox::log_init("./unit.conf");

OK($cfg = new GearboxConfigFile("./unit.conf"));

IS( $cfg->get_string("test-string"), "string" );
IS( $cfg->get_string("test-object", "test-string"),  "string" );
IS( $cfg->get_string_default("does-not-exist", "default"), "default");
IS( $cfg->get_string_default("does-not-exist", "does-not-exist", "default"),  "default");
IS( $cfg->get_string_default("test-object", "does-not-exist", "default"), "default");
IS( THROWS( function() use($cfg) { $cfg->get_string_default("test-int", "default"); } ), 
    'Json Exception: cannot convert integer to string at: ["test-int"]');

IS( $cfg->get_int("test-int"), 1 );
IS( $cfg->get_int("test-object", "test-int"), 1 );
IS( $cfg->get_int_default("does-not-exist", 42), 42 );
IS( $cfg->get_int_default("does-not-exist", "does-not-exist", 42), 42 );
IS( $cfg->get_int_default("test-object", "does-not-exist", 42), 42 );
IS( THROWS( function() use($cfg) { $cfg->get_int_default("test-string", 42); } ),
    'Json Exception: cannot convert string to integer at: ["test-string"]');

IS( $cfg->get_json("test-object"), array(
    "test-bool" => True,
    "test-double"  => 1.0,
    "test-int" => 1,
    "test-string" => "string"
  )
);

print "cols: " . exec("tput cols")  . "\n";
IS( $cfg->as_json(), array(
    "component" => "unit",
    "db_type"  => "sqlite3",
    "db_name"  => "./.db/test.db",
    "gearbox" => array(
        "conf" => "."
    ),
    "log"  => array(
        "config_file"  => "../../../../common/conf/stdout-logger.conf"
    ),
    "schemadir" => "./schemas",
    "status" => array(
        "persistence_type" => "transient"
    ),
    "scoreboard" => array(
        "name" => "./unit"
    ),
    "test-array" => array(
       "test"
    ),
    "test-bool" => True,
    "test-double"  => 1.0,
    "test-int" => 1,
    "test-object" => array(
        "test-bool" => True,
        "test-double"  => 1.0,
        "test-int" => 1,
        "test-string" => "string"
    ),
    "test-string" => "string"
));
?>
