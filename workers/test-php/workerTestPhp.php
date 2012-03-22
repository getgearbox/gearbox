<?php

require("Gearbox.php");

class PhpWorker extends Worker {
    function __construct( $config ) {
        parent::__construct( $config );
        $this->register_handler( "do_get_testphp_example_v1" );
        $this->register_handler( "do_post_testphp_example_v1" );
    }

    function do_post_testphp_example_v1( $job, $resp ) {
        $status = new Status( $resp->status() );
        $status->add_message("this is a status message!");
        throw new ERR_INTERNAL_SERVER_ERROR( "uh oh");
        return Worker::WORKER_SUCCESS;
    }

    function do_get_testphp_example_v1( $job, $resp ) {
        print "content: " . $job->content() . "\n";
        print "serialize: " . $job->serialize() . "\n";
        $i = 0;
        foreach ($job->arguments() as $arg ) {
          print "Arg $i: $arg\n";
          $i++;
        }

        foreach ( $job->matrix_arguments() as $key => $value ) {
          print "Matrix Arg: $key => $value\n";
        }
        
        foreach ( $job->query_params() as $key => $value ) {
          print "Query Param: $key => $value\n";
        }

        foreach ( $job->headers() as $key => $value ) {
          print "Header: $key => $value\n";
        }

        foreach ( $job->environ() as $key => $value ) {
          print "ENV: $key => $value\n";
        }
        
        print "status: " . $job->status() . "\n";
        print "name: " . $job->name() . "\n";
        print "base_uri: " . $job->base_uri() . "\n";

        print "type: ";
        switch ( $job->type() ) {
            case Job::JOB_UNKNOWN:
                print "UNKNOWN\n";
                break;
            case Job::JOB_ASYNC:
                print "ASYNC\n";
                break;
            case Job::JOB_SYNC:
                print "SYNC\n";
                break;
        } 

        print "api_version: " . $job->api_version() . "\n";
        print "operation: " . $job->operation() . "\n";
        print "component: " . $job->component() . "\n";
        print "resource_type: " . $job->resource_type() . "\n";
        print "resource_name: " . $job->resource_name() . "\n";
        print "resource_uri: " . $job->resource_uri() . "\n";
        print "remote_ip: " . $job->remote_ip() . "\n";
        print "remote_user: " . $job->remote_user() . "\n";
        print "timeout: " . $job->timeout() . "\n";

        print "resp code: " . $resp->code() . "\n";

        $status = $resp->status();
        $status->add_message("this is a status message!");
        print "status name: " . $status->name() . "\n";
        print "status resource_uri: " . $status->resource_uri() . "\n";
        print "status operation: " . $status->operation() . "\n";

        $resp->content('{"hello": "world"}');
        print "Done\n";
        return Worker::WORKER_SUCCESS;
    }
};

$worker = new PhpWorker( $argv[1] );
$worker->run();

?>
