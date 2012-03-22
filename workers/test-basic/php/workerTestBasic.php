<?php

require("Gearbox.php");

define("DBDIR", "/usr/var/gearbox/db/test-basic-php/");

class WorkerTestBasicPhp extends Worker {
  function __construct( $config ) {
    parent::__construct( $config );

    $this->register_handler( "do_get_testbasicphp_thing_v1" );
    $this->register_handler( "do_put_testbasicphp_thing_v1" );
    $this->register_handler( "do_post_testbasicphp_thing_v1" );
    $this->register_handler( "do_delete_testbasicphp_thing_v1" );
  }

  function do_get_testbasicphp_thing_v1( $job, $resp ) {
    if( array_key_exists( "TestBasic", $job->environ() ) ) {
      $env = $job->environ();
      $resp->add_header("TestBasic", $env["TestBasic"]);
    }

    $args = $job->arguments();
    if( empty( $args ) ) { // index GET
      $files = glob(DBDIR . "*");
      $out = array();

      // set things to an empty array in case our glob did not match anything
      $out["things"] = array();
      $limit = count($files);

      if( array_key_exists( "_count", $job->query_params() ) ) {
        $cgi = $job->query_params();
        $limit = intval( $cgi["_count"] );
      }

      for( $i=0; $i < $limit; $i++ ) {
        $matrix = $job->matrix_arguments();
        if( array_key_exists( "_expand", $matrix )
            && $matrix["_expand"] == "1" ) {
          $out["things"][$i] = json_decode(file_get_contents( $files[$i] ), true);
        }
        else {
          $out["things"][$i] = basename( $files[$i] );
        }
      }

      // set the output content
      $resp->content( json_encode($out) );
    }
    else {
      $name = $args[0];
      if( file_exists( DBDIR . $name ) ) {
        $resp->content( file_get_contents( DBDIR . $name ) );
      }
      else {
        throw new ERR_NOT_FOUND("thing \"" . $name . "\" not found");
      }
    }
    return Worker::WORKER_SUCCESS;
  }

  function do_put_testbasicphp_thing_v1( $job, $resp ) {
    // async message, so update status to let user know we are processing
    $resp->status()->progress(10);
    $resp->status()->add_message("processing");

    $args = $job->arguments();
    if( empty( $args ) ) {
      throw new ERR_BAD_REQUEST("missing required resource name");
    }

    $in = json_decode($job->content(), true);
    if( ! array_key_exists("id", $in) ) {
      throw new ERR_BAD_REQUEST("missing required \"id\" field");
    }

    file_put_contents( DBDIR . $args[0], $job->content() );
    $resp->status()->add_message("done");

    return Worker::WORKER_SUCCESS;
  }

  function do_post_testbasicphp_thing_v1( $job, $resp ) {
    $resp->status()->progress(10);
    $resp->status()->add_message("processing");

    # test SWIG wapper for scoreboarding
    Scoreboard::initialize( $resp->status()->component() );
    $sb = Scoreboard::get_scoreboard();
    $sb->increment_counter("status", "testbasicphp");

    $sw = new StopWatch();
    $sw->start();
    $sw->stop();

    $sb->update_duration_hours( "status", "testbasicphp", "duration_success", $sw );

    if( $job->operation() == "create" ) {
      // post-create where the resource id is created for user
      // (instead of a PUT where the user specifies the name)

      // get the generated id
      $in = json_decode($job->content(),true);
      $in["id"] = $job->resource_name();

      file_put_contents( DBDIR . $job->resource_name(), json_encode($in));
    }
    else {
      $args = $job->arguments();
      // post update
      if( file_exists( DBDIR . $args[0] ) ) {
        $in = json_decode($job->content(),true);
        $out = json_decode(file_get_contents(DBDIR . $args[0]), true);
        $out["stuff"] = $in["stuff"];
        file_put_contents( DBDIR . $args[0], json_encode($out) );
      }
      else {
        throw new ERR_NOT_FOUND("thing \"" . $args[0] . "\" not found");
      }
    }
    $resp->status()->add_message("done");
    return Worker::WORKER_SUCCESS;
  }

  function do_delete_testbasicphp_thing_v1( $job, $resp ) {
    $resp->status()->progress(10);
    $resp->status()->add_message("processing");

    // don't actually delete if fake-out header is set
    $headers = $job->headers();
    if( array_key_exists("fake-out", $headers)
        && intval($headers["fake-out"]) ) {
      $resp->status()->add_message("ignoring delete due to fake-out header");
      return Worker::WORKER_SUCCESS;
    }

    $args = $job->arguments();
    if( empty( $args ) ) {
      throw new ERR_BAD_REQUEST("missing required resource name");
    }

    $file = DBDIR . $args[0];
    if( file_exists($file) && is_file($file) ) {
      unlink($file);
    }
    else {
      throw new ERR_NOT_FOUND("thing \"" . $args[0] . "\" not found");
    }

    $resp->status()->add_message("done");
    return Worker::WORKER_SUCCESS;
  }
};

$worker = new WorkerTestBasicPhp( $argv[1] );
$worker->run();

?>
