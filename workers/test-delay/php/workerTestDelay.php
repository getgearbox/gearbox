<?php

require("Gearbox.php");

define("DBDIR", "/usr/var/gearbox/db/test-delay-php/");

class WorkerTestDelayPhp extends Worker {
  function __construct( $config ) {
    parent::__construct( $config );
    
    $this->register_handler( "do_get_testdelayphp_counter_v1" );
    $this->register_handler( "do_post_testdelayphp_counter_v1" );
    $this->register_handler( "do_delete_testdelayphp_counter_v1" );
    $this->register_handler( "do_increment_testdelayphp_counter_v1" );
  }

  function do_get_testdelayphp_counter_v1( $job, $resp ) {
    $resp->content( file_get_contents( DBDIR . $job->resource_name() ) );
    return Worker::WORKER_SUCCESS;
  }

  function do_post_testdelayphp_counter_v1( $job, $resp ) {
    $matrix = $job->matrix_arguments();
    $start = 0;
    if( array_key_exists("start", $matrix) ) {
      $start = $matrix["start"];
    }
    file_put_contents( DBDIR . $job->resource_name(), $start);

    $seconds = 1;
    if( array_key_exists("delay", $matrix) ) {
      $seconds = $matrix["delay"];
    }
    $this->afterwards( $job, "do_increment_testdelayphp_counter_v1", intval($seconds) );
    return Worker::WORKER_CONTINUE;
  }

  function do_delete_testdelayphp_counter_v1( $job, $resp ) {
    $args = $this->arguments();
    unlink(DBDIR . $args[0]);
    return Worker::WORKER_SUCCESS;
  }
  
  function do_increment_testdelayphp_counter_v1($job,$resp) {
    $newval = 1 + file_get_contents(DBDIR . $job->resource_name());
    file_put_contents( DBDIR . $job->resource_name(), $newval );
    $matrix = $job->matrix_arguments();
    $start = 0;
    if( array_key_exists("start", $matrix) ) {
      $start = $matrix["start"];
    }
    $end = 10;
    if( array_key_exists("end", $matrix) ) {
      $end = $matrix["end"];
    }
    $resp->status()->add_message("set to " . $newval);
    if( $newval == $end ) {
      return Worker::WORKER_SUCCESS;
    }
    else {
      $resp->status()->progress( $resp->status()->progress() + ($end - $start) );
    }
    $seconds = 1;
    $matrix = $job->matrix_arguments();
    if( array_key_exists("delay", $matrix) ) {
      $seconds = $matrix["delay"];
    }

    if ( array_key_exists("retry", $matrix) && $matrix["retry"] ) {
      $resp->status()->add_message("retry attempt number " . ($resp->status()->failures()+1));
      return Worker::WORKER_RETRY;
    } else {
      $this->afterwards($job, intval($seconds));
    }
    return Worker::WORKER_CONTINUE;
  }
};

$worker = new WorkerTestDelayPhp( $argv[1] );
$worker->run();

?>
