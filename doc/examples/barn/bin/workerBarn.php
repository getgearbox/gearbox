<?php

require("Gearbox.php");

define("DBDIR", "/home/y/var/gearbox/db/barn/");

class Animal extends Worker {
    function __construct($config) {
        parent::__construct($config);
    
        $this->register_handler("do_get_barn_animal_v1");
        $this->register_handler("do_put_barn_animal_v1");
        $this->register_handler("do_post_barn_animal_v1");
        $this->register_handler("do_delete_barn_animal_v1");
    }
  
    function do_get_barn_animal_v1($job, $resp) {
        $args = $job->arguments();
         
        if (empty($args)) {
            chdir(DBDIR. "animals");
            $animals = glob("*");
            $out['animals'] = $animals === false ? array() : $animals;
        }
        else {
            $out = $this->get_animal($args[0]);
        }
        $resp->content(json_encode($out));
        return Worker::WORKER_SUCCESS;
    }
   
    function get_animal($id) {
        if (file_exists(DBDIR . "animals/$id")) {
            return json_decode(file_get_contents(DBDIR . "animals/$id"), true);
        }
        else {
            throw new ERR_NOT_FOUND("Sorry, the barn does not contain $id.");
        }
    }
 
    function do_put_barn_animal_v1($job, $resp) {
        $args = $job->arguments();
        
        if (empty($args)) {
            throw new ERR_BAD_REQUEST("Your URL must include a valid animal ID.");
        }
        
        file_put_contents(DBDIR . "animals/$args[0]", $job->content());

        $this->do_work($args[0], $resp);
        return Worker::WORKER_SUCCESS;
    }
    
    function do_post_barn_animal_v1($job, $resp) {
        $animal = json_decode($job->content(), true);

        if ($job->operation() == "create") {
            if (! isset($animal['species'])) {
                $animal['species'] = "Sus domestica";
            }
            $id = $job->resource_name();
            file_put_contents(DBDIR . "animals/$id", json_encode($animal));
        }
        else { 
            $args = $job->arguments();
            $id = $args[0];
            $out = $this->get_animal($id); 
            if (isset($animal['species'])) {
                $out["species"] = $animal["species"];
            }
            file_put_contents(DBDIR . "animals/$id", json_encode($out));
        }

        $this->do_work($id, $resp);
        return Worker::WORKER_SUCCESS;
    }
    
    function do_delete_barn_animal_v1($job, $resp) {
        $args = $job->arguments();
        
        if (empty($args)) {
            throw new ERR_BAD_REQUEST("Your URL must include a valid animal ID.");
        }
        
        if (file_exists(DBDIR . "animals/$args[0]")) {
           unlink(DBDIR . "animals/$args[0]"); 
        }
        else {
           throw new ERR_NOT_FOUND("Sorry, the barn does not contain $args[0].");
        }
        
        $resp->status()->add_message("$args[0] is now dead. Good job!");
        return Worker::WORKER_SUCCESS;
    }
   
    function do_work($id, $resp) {
        $animal = $this->get_animal($id);
        if ($animal['species'] == 'Araneus cavaticus') {
            $this->spin_web($animal['name'], $resp);
        }
        else {
            $resp->status()->add_message($animal['name'] . " is happily rolling around in the mud!");        
        }

    }

    function spin_web($name, $resp) {
        $resp->status()->progress(10);
        $resp->status()->add_message("$name is beginning to spin a web.");
        sleep(30);
        $resp->status()->progress(50);
        $resp->status()->add_message("$name is still spinning...");
        sleep(30);

        $messages = array('SOME PIG', 'TERRIFIC', 'RADIANT', 'HUMBLE');
        $message = $messages[array_rand($messages)];
        $resp->status()->add_message("$name's web is complete! It says: '$message'");
    } 
}

$worker = new Animal($argv[1]);
$worker->run();

?>
