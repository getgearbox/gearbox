#!/usr/bin/env phprun -s
<?

require_once("Gearbox.php");
require_once("test-trivial.php");

PLAN(39);

class TestWorker extends GearboxWorker {
  function __construct( $config ) {
    parent::__construct( $config );
    
    // verify 3 forms of register_handler 
    // work.  Each handler will check api_version of 
    // incoming job to make sure it matches our api
    
    $self = $this;

    NOTHROW( function() use($self) {
      $self->register_handler(
        "do_run_handler_thing_v1",
        function($job,$resp) {
          IS( $job->api_version(), "v1" );
          return GearboxWorker::WORKER_SUCCESS;
        }
      ); 
    } );

    NOTHROW( function() use($self) { $self->register_handler( "do_run_handler_thing_v2", "handler" ); } );
    NOTHROW( function() use($self) { $self->register_handler( "do_run_handler_thing_v3" ); } );
  }
  
  function handler($job, $resp) {
    IS( $job->api_version(), 'v2' );
    return GearboxWorker::WORKER_SUCCESS;
  }
  
  function do_run_handler_thing_v3($job, $resp) {
    IS( $job->api_version(), 'v3' );
    return GearboxWorker::WORKER_SUCCESS;
  }
  
  // this is the order we will run the jobs so that we can test the names
  // in the pre/post request handlers
  private $queue = array('do_run_handler_thing_v1', 'do_run_handler_thing_v2', 'do_run_handler_thing_v3');

  // these get called automatically when a job comes in, verify the job
  // name is correct and verify no exceptions from parent pre/post request handlers
  function pre_request($job) {
    IS( $job->name(), $this->queue[0] );
    parent::pre_request($job);
  }

  function post_request($job) {
    IS( $job->name(), array_shift($this->queue) );
    parent::post_request($job);
  }

}

OK( $worker = new TestWorker("./unit.conf") );

OK( $jm = $worker->job_manager() );
NOTHROW( function() use($jm) { $jm->base_uri("http://localhost:4080/test/v1"); } );

OK( $sm = $worker->status_manager() );
NOTHROW( function() use($sm) { $sm->base_uri("http://localhost:4080/test/v1"); } );

// test contstants
IS( GearboxWorker::WORKER_SUCCESS, 0 );
IS( GearboxWorker::WORKER_ERROR, 1 );
IS( GearboxWorker::WORKER_CONTINUE, 2 );
IS( GearboxWorker::WORKER_RETRY, 3 );

// reset max_requests so that we can test the worker::run
// (otherwise it is an infinate loop)

NOTHROW( function() use($worker) { $worker->max_requests(1); } );
IS( $worker->request_count(), 0 );

// create v1 job, add it to queue, then let worker run it
OK( $job = $jm->job("do_run_handler_thing_v1") );
NOTHROW( function() use($job) { $job->run(); } );
NOTHROW( function() use($worker) { $worker->run(); } );

// update to v2 job, add it to queue, then let worker run it
NOTHROW( function() use($job) { $job->name("do_run_handler_thing_v2")->run(); } );
NOTHROW( function() use($worker) { $worker->run(); } );

// update to v3 job, add it to queue, then let worker run it
NOTHROW( function() use($job) { $job->name("do_run_handler_thing_v3")->run(); } );
NOTHROW( function() use($worker) { $worker->run(); } );

// we have handled requests, count should be updated
IS( $worker->request_count(), 3 );

// verify aftewards jobs dont throw exceptions
NOTHROW( function() use($worker, $job) { $worker->afterwards( $job ); } );
NOTHROW( function() use($worker, $job) { $worker->afterwards( $job, 30 ); } );

OK( $cfg = $worker->cfg() );
IS( $cfg->get_string("component"), "unit" );

//
// assign random data members of worker class
//

OK( $worker->a = 1 );
IS( $worker->a, 1 );
OK( $worker->b = 2 );
IS( $worker->b, 2 );

?>
