#!/usr/bin/env phprun -s # -*- mode: php-*-
<?
require_once("Gearbox.php");
require_once("test-trivial.php");

PLAN(80);

Gearbox::log_init("./unit.conf");

IS( GearboxStatus::state2str(GearboxStatus::STATE_UNKNOWN), "UNKNOWN");
IS( GearboxStatus::state2str(GearboxStatus::STATE_PENDING), "PENDING");
IS( GearboxStatus::state2str(GearboxStatus::STATE_RUNNING), "RUNNING");
IS( GearboxStatus::state2str(GearboxStatus::STATE_STOPPING), "STOPPING");
IS( GearboxStatus::state2str(GearboxStatus::STATE_STOPPED), "STOPPED");
IS( GearboxStatus::state2str(GearboxStatus::STATE_CANCELLING), "CANCELLING");
IS( GearboxStatus::state2str(GearboxStatus::STATE_COMPLETED), "COMPLETED");

IS( GearboxStatus::str2state("UNKNOWN"), GearboxStatus::STATE_UNKNOWN);
IS( GearboxStatus::str2state("PENDING"), GearboxStatus::STATE_PENDING);
IS( GearboxStatus::str2state("RUNNING"), GearboxStatus::STATE_RUNNING);
IS( GearboxStatus::str2state("STOPPING"), GearboxStatus::STATE_STOPPING);
IS( GearboxStatus::str2state("STOPPED"), GearboxStatus::STATE_STOPPED);
IS( GearboxStatus::str2state("CANCELLING"), GearboxStatus::STATE_CANCELLING);
IS( GearboxStatus::str2state("COMPLETED"), GearboxStatus::STATE_COMPLETED);

IS( GearboxStatus::event2str(GearboxStatus::EVENT_UNKNOWN), "UNKNOWN");
IS( GearboxStatus::event2str(GearboxStatus::EVENT_PRECANCEL), "PRECANCEL");
IS( GearboxStatus::event2str(GearboxStatus::EVENT_CANCEL), "CANCEL");

IS( GearboxStatus::str2event("UNKNOWN"), GearboxStatus::EVENT_UNKNOWN);
IS( GearboxStatus::str2event("PRECANCEL"), GearboxStatus::EVENT_PRECANCEL);
IS( GearboxStatus::str2event("CANCEL"), GearboxStatus::EVENT_CANCEL);

OK( $sm = new GearboxStatusManager(new GearboxConfigFile("./unit.conf")));
$BASE = "http://localhost:4080/transient/";
NOTHROW( function() use($sm) { $sm->base_uri($BASE); });

OK( $p = $sm->create("s-0", "operation", "$BASE/thing", "component"));
OK( $s = $sm->create("s-1", "operation", "$BASE/thing", "component"));
IS( $s->name(), "s-1");
IS( $s->operation(), "operation");
IS( $s->resource_uri(), "$BASE/thing");
IS( $s->component(), "component");
IS( $s->uri(), "http://localhost:4080/transient/status/s-1");

IS( $sm->fetch("s-1")->serialize(), $s->serialize() );
IS( $sm->fetch(json_decode($s->serialize(), true))->serialize(), $s->serialize() );

# setup up parent
NOTHROW( function() use($s,$p) { $s->parent_uri($p->uri()); });
IS( $s->parent_uri(), $p->uri());
IS( $s->parent()->serialize(), $p->serialize());

# test message apis
IS( $s->messages(), array());
NOTHROW( function() use($s) { $s->add_message("message"); });
IS( $s->messages(), array("message"));

# test children apis
IS( $s->children(), array());
NOTHROW( function() use($s, $BASE) { $s->add_child("$BASE/status/s-2"); });
IS( $s->children(), array("$BASE/status/s-2"));

# test meta apis
IS( $s->meta(), null);
NOTHROW( function() use($s) { $s->meta(array("key" => "value")); });
IS( $s->meta(), array("key" => "value"));
NOTHROW( function() use($s) { $s->meta(array(a=>1, b=>2)); });
IS( $s->meta(), array(a=>1, b=>2));

# test progress apis
IS( $s->progress(), 0);
NOTHROW( function() use($s) { $s->progress(50); });
IS( $s->progress(), 50);

# fail the status, set progress, code and state
NOTHROW( function() use($s) { $s->fail(123); });
IS( $s->progress(), 100);
IS( $s->code(), 123);
IS( $s->state(), GearboxStatus::STATE_COMPLETED);

# set to successful, progress, code and state updated
NOTHROW( function() use($s) { $s->success(); });
IS( $s->progress(), 100);
IS( $s->code(), 0);
IS( $s->state(), GearboxStatus::STATE_COMPLETED);

NOTHROW( function() use($s) { $s->checkpoint(); });

# test cancel api
NOTHROW( function() use($s) { $s->state(GearboxStatus::STATE_CANCELLING); });
NOTHROW( function() use($s) { $s->cancel(); });
IS( $s->code(), 0);
IS( $s->state(), GearboxStatus::STATE_CANCELLED);

IS( THROWS( function() use($s) { $s->checkpoint(); } ), "Worker Stop");

OK( $jm = new GearboxJobManager(new GearboxConfigFile("./unit.conf")));
OK( $job = $jm->job("do_get_component_other_v1"));

NOTHROW( function() use($s,$job) { $s->on(GearboxStatus::EVENT_PRECANCEL, $job); });
IS( $s->on(GearboxStatus::EVENT_PRECANCEL, $jm)->name(), $job->name());

NOTHROW( function() use($s, $job) { $s->on(GearboxStatus::EVENT_CANCEL, $job); });
IS( $s->on(GearboxStatus::EVENT_CANCEL, $jm)->name(), $job->name());

OK( $s->ctime() <= time());
OK( $s->mtime() <= time());

IS( $s->ytime(), -1);
NOTHROW( function() use($s) { $s->ytime(0); });
IS( $s->ytime(), 0);

NOTHROW( function() use($s) { $s->sync(); });

IS( $s->has_completed(), True);
OK( $s->is_success());

IS( $s->failures(), 0);
NOTHROW( function() use($s) { $s->failures(1); });
IS( $s->failures(), 1);

IS( json_decode($s->serialize(),true), array(
    'children' => array(
        'http://localhost:4080/transient/status/s-2'
    ),
    'code' => 0,
    'component' => 'component',
    'concurrency' => 0,
    'ctime' => $s->ctime(),
    'failures' => 1,
    'messages' => array(
        'message'
    ),
    'meta' => array(
        'a' => 1,
        'b' => 2
    ),
    'mtime' => $s->mtime(),
    'operation' => 'operation',
    'parent_uri' => 'http://localhost:4080/transient/status/s-0',
    'progress' => 100,
    'uri' => 'http://localhost:4080/transient//thing',
    'state' => 'CANCELLED',
    'status_uri' => 'http://localhost:4080/transient/status/s-1'
));
?>
