<?php

require_once("SwigGearbox.php");
require_once("GearboxErrors.php");

class GearboxWorker extends GearboxPhpWorker {
  function __construct( $config ) {
    parent::__construct( $config );
    $this->set_self($this);
  }

   function do_dispatch( $job, $resp ) {
     $method = $job->name();
     if( array_key_exists($method, $this->handlers) ) {
       $method = $this->handlers[$method];
     }
     if( !is_string($method) ) {
       return $method($job,$resp);
     }
     
     if( method_exists($this, $method) ) {
       return call_user_func( array($this, $method), $job, $resp );
     }
     // we should never get here ... someone registered a function
     // that does not exist!
     throw new ERR_INTERNAL_SERVER_ERROR("job ". $method ." does not exists in class " . get_class($this));
   }

   // this overload is here because swig gets it way wrong in
   // the generated SwigGearbox and always dispatches to the 3
   // arg afterwards()
   function afterwards($job,$name_or_delay=null,$delay=null) {
     switch (func_num_args()) {
     case 1: SwigWorker_afterwards($this->_cPtr,$job); break;
     case 2: SwigWorker_afterwards($this->_cPtr,$job,$name_or_delay); break;
     default: SwigWorker_afterwards($this->_cPtr,$job,$name_or_delay,$delay); break;
     }
   }
};

class GearboxJobManager extends GearboxRealJobManager {
  function job_queue_apply($queue, $function, $data) {
    foreach( $queue as $level ) {
      foreach ( $level as $job ) {
        if( method_exists($job, $function) ) {
          call_user_func( array($job,$function), $data );
        }
        else {
          throw new ERR_INTERNAL_SERVER_ERROR("object of type ".get_class($job)." does not have member function $function");
        }
      }
    }
  }
}

?>
