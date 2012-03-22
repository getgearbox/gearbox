<?php

require("Gearbox.php");

define("DBDIR", "/usr/var/gearbox/db/test-chained-php/");

class WorkerTestBasicPhp extends Worker {
  function __construct( $config ) {
    parent::__construct( $config );

    $this->register_handler( "do_get_testchainedphp_hello_v1" );
    $this->register_handler( "do_get_internalphp_hello1_v1" );
    $this->register_handler( "do_post_testchainedphp_hello2_v1" );

    $this->register_handler( "do_get_testchainedphp_goodbye_v1" );
    $this->register_handler( "do_post_testchainedphp_goodbye_v1" );
    $this->register_handler( "do_append_internalphp_goodbye1_v1" );
    $this->register_handler( "do_append_internalphp_goodbye2_v1" );

    $this->register_handler( "do_get_testchainedphp_thing_v1" );
    $this->register_handler( "do_post_testchainedphp_thing_v1" );
    $this->register_handler( "do_reg_internalphp_service1_v1" );
    $this->register_handler( "do_post_testchainedphp_service2_v1" );

    $this->register_handler( "do_delete_testchainedphp_thing_v1" );
    $this->register_handler( "do_unreg_internalphp_service1_v1" );
    $this->register_handler( "do_delete_testchainedphp_service2_v1" );
  }

  // I am not sure why we would want to do a chained syncronous get, but
  // you can chain a bunch of sync jobs together
  function do_get_testchainedphp_hello_v1( $job, $resp ) {

    $content = json_encode("Hello from job");

    // do internal hello1 which just appends it name to our content
    $j = new Job($job);
    $j->name("do_get_internalphp_hello1_v1");
    $j->type(Job::JOB_SYNC);
    $j->content($content);
    $r = $j->run();

    // create sync http rest job back to localhost which takes
    // the output from previous job and adds its own name.
    $j = $this->job_manager()->job(HttpClient::METHOD_POST, $job->base_uri()."/hello2");
    $j->content($r->content());
    $j->headers($r->headers());
    $r = $j->run();

    $resp->content( $r->content() );
    $resp->headers( $r->headers() );
    return Worker::WORKER_SUCCESS;
  }

  function do_get_internalphp_hello1_v1( $job, $resp ) {
    $in = json_decode($job->content(), true);
    $in .= " and job1";
    $resp->add_header("job1-header", "1");
    $resp->content(json_encode($in));
    return Worker::WORKER_SUCCESS;
  }

  // this is a SYNC post call configured via the httpd-test-chained.conf
  function do_post_testchainedphp_hello2_v1( $job, $resp ) {
    $in = json_decode($job->content(), true);
    $in .= " and job2";
    $resp->headers($job->headers());
    $resp->add_header("job2-header", "1");
    $resp->content(json_encode($in));
    return Worker::WORKER_SUCCESS;
  }

  function do_get_testchainedphp_goodbye_v1( $job, $resp ) {
    $resp->content( file_get_contents( DBDIR . $job->resource_name() ) );
    return Worker::WORKER_SUCCESS;
  }

  function do_post_testchainedphp_goodbye_v1( $job, $resp ) {
    $resp->status()->add_message("processing from " . $job->name());
    $content = json_encode("Goodbye from job");
    file_put_contents( DBDIR . $job->resource_name(), $content );

    // do internal goodbye1 which just appends its name to our content
    $this->afterwards($job, "do_append_internalphp_goodbye1_v1");
    // don't finalize the status, are going to keep going
    return Worker::WORKER_CONTINUE;
  }

  function do_append_internalphp_goodbye1_v1( $job, $resp ) {
    $resp->status()->add_message("processing from " . $job->name());
    $content = json_decode(file_get_contents( DBDIR . $job->resource_name() ), true);
    $content .= " and job1";
    file_put_contents( DBDIR . $job->resource_name(), json_encode($content) );

    // do internal goodbey2 which just appends it name to our content
    $this->afterwards($job, "do_append_internalphp_goodbye2_v1");
    // don't finalize the status, are going to keep going
    return Worker::WORKER_CONTINUE;
  }

  function do_append_internalphp_goodbye2_v1( $job, $resp ) {
    $resp->status()->add_message("processing from " . $job->name());
    $content = json_decode( file_get_contents( DBDIR . $job->resource_name() ) );
    $content .= " and job2";
    file_put_contents( DBDIR . $job->resource_name(), json_encode($content) );

    // finally done so dont continue
    return Worker::WORKER_SUCCESS;
  }



  function do_get_testchainedphp_thing_v1( $job, $resp ) {
    $resp->content( file_get_contents( DBDIR . $job->resource_name() ) );
    return Worker::WORKER_SUCCESS;
  }

  function do_post_testchainedphp_thing_v1( $job, $resp ) {
    $resp->status()->add_message("processing from " . $job->name());
    $out = array();
    $out["id"] = $job->resource_name();
    file_put_contents( DBDIR . $job->resource_name(), json_encode($out) );

    // our new thing needs to be registered with 2 fancy
    // services.  They can both be registered at the same
    // time in parallel.
    $jm = $this->job_manager();

    $responses = array();

    // service 1 is registered via async local worker
    array_push($responses, $jm->job("do_reg_internalphp_service1_v1")->content(json_encode($out))->run());

    array_push($responses, $jm->job(HttpClient::METHOD_POST, $job->base_uri() . "/service2")->content(json_encode($out))->run());

    while ( ! empty($responses) ) {
      $s = $responses[0]->status();

      $sm = $this->status_manager();
      $status = $sm->fetch($s->uri());

      $s->sync();
      if( $s->has_completed() ) {
        if( $s->is_success() ) {
          array_shift($responses);
        }
        else {
          $ERR = "ERR_CODE_".$s->code();
          $msgs = $s->messages();
          throw new $ERR($msgs[0]);
        }
        // pause between polling again
        sleep(1);
      }
    }
    return Worker::WORKER_SUCCESS;
  }

  function do_reg_internalphp_service1_v1($job, $resp) {
    $resp->status()->add_message("service1 registered");
    return Worker::WORKER_SUCCESS;
  }

  function do_post_testchainedphp_service2_v1($job, $resp) {
    $resp->status()->add_message("service2 registered");
    return Worker::WORKER_SUCCESS;
  }

  function do_delete_testchainedphp_thing_v1( $job, $resp ) {
    // our new thing needs to be unregistered with 2 fancy
    // services.  service 1 must be unregistered before service 2

    $content = file_get_contents( DBDIR . $job->resource_name() );

    $jm = $this->job_manager();

    $jobs = array();
    array_push( $jobs, array() );
    array_push( $jobs, array() );

    // first gen jobs only has service1, unregister happens via local worker
    array_push( $jobs[0], $jm->job("do_unreg_internalphp_service1_v1") );

    // second gen jobs only has service 2, unregister happens via DELETE
    // http call on remote worker
    array_push( $jobs[1], $jm->job(HttpClient::METHOD_DELETE, $job->base_uri() . "/service2"));

    $jm->job_queue_apply($jobs, "content", $content);

    $jm->job_queue_run($jobs);
    return Worker::WORKER_SUCCESS;
  }

  function do_unreg_internalphp_service1_v1($job, $resp) {
    $resp->status()->add_message("service1 unregistered");
    return Worker::WORKER_SUCCESS;
  }

  function do_delete_testchainedphp_service2_v1($job, $resp) {
    $resp->status()->add_message("service2 unregistered");
    return Worker::WORKER_SUCCESS;
  }
}

$worker = new WorkerTestBasicPhp( $argv[1] );
$worker->run();

?>
