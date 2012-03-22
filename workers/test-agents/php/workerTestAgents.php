<?php

require("Gearbox.php");

define("DBDIR", "/var/gearbox/db/test-agents-php/");

class WorkerTestAgentsPhp extends Worker {
  function __construct( $config ) {
    parent::__construct( $config );
    
    $this->register_handler( "do_get_testagentsphp_thing_v1", "thing_handler" );
    $this->register_handler( "do_post_testagentsphp_thing_v1", "thing_handler");
    $this->register_handler( "do_delete_testagentsphp_thing_v1", "thing_handler");
    
    foreach( array("A", "B", "C", "D") as $resource ) {
      foreach( array( "reg", "unreg" ) as $op ) {
        $this->register_handler( "do_${op}_testagentsphp_${resource}_v1", "dummy_handler" );
      }
    }
  }
  
  function thing_handler ( $job, $resp ) {
    $content = "";
    if( is_file(DBDIR . $job->resource_name()) ) { 
      $content = json_decode(file_get_contents( DBDIR . $job->resource_name() ), true);
    }

    if( $job->operation() == "get" ) {
      $resp->content(json_encode($content));
      return Worker::WORKER_SUCCESS;
    }

    $agents = json_decode(file_get_contents("/etc/gearbox/test-agents-php-agents.conf"), true);

    $resp->status()->add_message("calling agents");
    
    if( $job->operation() == "create" ) {
      $content = array();
      $content["id"] = $job->resource_name();
      file_put_contents(DBDIR.$job->resource_name(), json_encode($content));
      
      $run_agents = $this->job_manager()->job("do_run_global_agents_v1");
      $agents_content = array();
      $agents_content["agents"] = $agents["register"];
      $agents_content["content"] = json_encode($content);
      $run_agents->content( json_encode( $agents_content ) );
      $r = $run_agents->run();
      $s = $r->status();
      // poll for agents to be done
      do {
        sleep(1);
        $s->sync();
      } while( ! $s->has_completed() );
      
      if ( ! $s->is_success() ) {
        $ERR = "ERR_CODE_" . $s->code();
        $msg = $s->message();
        throw new $ERR( $msg[ count($msg) - 1 ] );
      }
    }
    else {
      // operation == delete
      $jm = $this->job_manager();
      $queue = $jm->job_queue( $agents["unregister"] );
      $jm->job_queue_apply($queue, "content", json_encode($content));
      $jm->job_queue_run($queue);
      unlink( DBDIR . $job->resource_name() );
    }

    $resp->status()->add_message("done");
    return Worker::WORKER_SUCCESS;
  }

  function dummy_handler ($job, $resp) {
    $in = json_decode($job->content(), "true");
    $msg  = $job->resource_type() . " ";
    $msg .= $job->operation() == "reg" ? "registered" : "unregistered";
    $msg .= " for " . $in["id"];
    $resp->status()->add_message($msg);
    // give us time from smoke tests to verify the progress of the
    // agents job
    if( $job->operation() == "reg" ) {
        sleep(10);
    }

    $resp->status()->meta( $resp->status()->name(), $in["id"] );
    return Worker::WORKER_SUCCESS;
  }
};
  
$worker = new WorkerTestAgentsPhp( $argv[1] );
$worker->run();

?>
