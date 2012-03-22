<?php

require("Gearbox.php");

define("DBDIR", "/usr/var/gearbox/db/test-cancel-php/");

class WorkerTestCancelPhp extends Worker {
  function __construct( $config ) {
    parent::__construct( $config );
    
    $this->register_handler( "do_post_testcancelphp_thing_v1" );
    $this->register_handler( "do_cancel_testcancelphp_thing_v1" );

    $this->register_handler( "do_post_testcancelphp_continuation_v1" );
    $this->register_handler( "do_run_testcancelphp_continuation_v1" );
    $this->register_handler( "do_finish_testcancelphp_continuation_v1" );
  }

  function do_post_testcancelphp_thing_v1( $job, $resp ) {
    $s = $resp->status();
    $onCancel = $this->job_manager()->job("do_cancel_testcancelphp_thing_v1");
    $s->on(Status::EVENT_CANCEL, $onCancel);

    $stop = time() + 30;
    while($stop >= time()) {
      $s->sync();
      $p = $s->progress();
      print_r($p);
      if( $p < 100 ) {
        $p += 10;
        print_r($p);
        $s->progress($p);
        $s->checkpoint();
        sleep(5);
      }
      else {
        return Worker::WORKER_SUCESS;
      }
    }
    return Worker::WORKER_ERROR;
  }

  function do_cancel_testcancelphp_thing_v1( $job, $resp ) {
    $resp->status()->add_message("on cancel callback called");
    return Worker::WORKER_SUCCESS;
  }

  function do_post_testcancelphp_continuation_v1( $job, $resp ) {
    // we have do_post with a continuation of do_finish.  The do_finish
    // is only called via on-completion handler for do_run
    $run = $this->job_manager()->job("do_run_testcancelphp_continuation_v1");
    $finish = new Job($job);
    $finish->name("do_finish_testcancelphp_continuation_v1");
    $run->on(Job::EVENT_COMPLETED, $finish);
    $this->afterwards($run);
    return Worker::WORKER_CONTINUE;
  }

  function do_run_testcancelphp_continuation_v1( $job, $resp ) {
    $resp->status()->add_message("run called");
    // this will retry indefinately, we want to test the cancellation of an
    // status in progress that is suspended waiting upon child completion events
    return Worker::WORKER_RETRY;
  }

  function do_finish_testcancelphp_continuation_v1( $job, $resp ) {
    // this will never get called since do_run will never complete
    return Worker::WORKER_SUCCESS;
  }
};

$worker = new WorkerTestCancelPhp( $argv[1] );
$worker->run();

?>
