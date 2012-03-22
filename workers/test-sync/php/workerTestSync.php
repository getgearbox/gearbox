<?php

require("Gearbox.php");

define("DBDIR", "/usr/var/gearbox/db/test-sync-php/");

class WorkerTestSyncPhp extends Worker {
  function __construct( $config ) {
    parent::__construct( $config );
    
    $this->register_handler( "do_get_testsyncphp_thing_v1" );
    $this->register_handler( "do_put_testsyncphp_thing_v1" );
    $this->register_handler( "do_post_testsyncphp_thing_v1" );
    $this->register_handler( "do_delete_testsyncphp_thing_v1" );
  }
  
  function do_get_testsyncphp_thing_v1( $job, $resp ) {
    if( array_key_exists("TestSync", $job->environ() ) ) {
      $env = $job->environ();
      $resp->add_header("TestSync", $env["TestSync"]);
    }
    
    $args = $job->arguments();
    if( empty( $args ) ) {
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
  
  function do_put_testsyncphp_thing_v1( $job, $resp ) {
    $args = $job->arguments();
    if( empty( $args ) )
      throw new ERR_BAD_REQUEST("missing required resource name");
    
    $in = json_decode( $job->content(), true );
    if( ! array_key_exists("id", $in) ) {
      throw new ERR_BAD_REQUEST("missing required \"id\" field");
    }
    
    file_put_contents( DBDIR . $args[0], $job->content() );
    
    $resp->content( $job->content() );
    return Worker::WORKER_SUCCESS;
  }

  function do_post_testsyncphp_thing_v1( $job, $resp ) {
    if( $job->operation() == "create" ) {
      // post-create where the resource id is created for user
      // (instead of a PUT where the user specifies the name)
        
      // get the generated id
      $in = json_decode($job->content(),true);
      $in["id"] = $job->resource_name();

      $content = json_encode($in);
      file_put_contents( DBDIR . $job->resource_name(), $content);
      $resp->content($content);
    }
    else { 
      $args = $job->arguments();
      // post update
      if( file_exists( DBDIR . $args[0] ) ) {
        $in = json_decode($job->content(),true);
        $out = json_decode(file_get_contents(DBDIR . $args[0]), true);
        $out["stuff"] = $in["stuff"];
        $content = json_encode($out);
        file_put_contents( DBDIR . $args[0], $content );
        $resp->content($content);
      }
      else {
        throw new ERR_NOT_FOUND("thing \"" . $args[0] . "\" not found");
      }
    }
    return Worker::WORKER_SUCCESS;
  }

  function do_delete_testsyncphp_thing_v1( $job, $resp ) {
    // don't actually delete if fake-out header is set
    $headers = $job->headers();
    if( array_key_exists("fake-out", $headers)
        && intval($headers["fake-out"]) ) {
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
    return Worker::WORKER_SUCCESS;
  }
};

$worker = new WorkerTestSyncPhp( $argv[1] );
$worker->run();

?>
