<?
require_once("Test-More-OO.php");

$tz = readlink("/etc/localtime");
if( preg_match("|([^/]+/[^/]+)$|", $tz, $match) ) {
  date_default_timezone_set($match[1]);
}

global $TestTrivialMaxCols;
$TestTrivialMaxCols = exec("tput cols") - 30;

class TestTrivial extends TestMore {
  private $file_cache = array();
  protected $level = 0;
  protected $CurrentTestNumber = 0;

  protected $OPS = array(
                         'EQ'     => "==",
                         'ID'     => "==",
                         'IS'     => "==",
                         'ISA'    => "ISA",
                         'ISNT'   => "!=",
                         'LIKE'   => "=~",
                         'UNLIKE' => "!~",
                         );
  
  function level ( $newLevel ) {
    $this->level = $newLevel;
  }
  
  function OK($test, $msg=NULL) {
    if( !isset($msg) ) {
      $msg = $this->line_to_text($this->level + 1);
    }
    return parent::ok($test, $msg);
  }

  function NOK($test, $msg=NULL) {
    if( !isset($msg) ) {
      $msg = $this->line_to_text($this->level + 1);
    }
    return parent::ok(!$test, $msg);
  }

  function IS($lhs, $rhs, $msg=NULL) {
    if( !isset($msg) ) {
      $msg = $this->line_to_text($this->level + 1);
    }
    return parent::is_deeply($lhs, $rhs, $msg);
  }

  function LIKE($string, $pattern, $msg=NULL) {
    if( !isset($msg) ) {
      $msg = $this->line_to_text($this->level + 1);
    }
    return parent::like($string,$pattern,$msg);
  }

  function THROWS($func) {
    try {
      $func();
    }
    catch( Exception $e ) {
      return $e->getMessage();
    }
  }

  function NOTHROW($func, $msg=NULL) {
    if( !isset($msg) ) {
      $msg = $this->line_to_text($this->level + 1);
    }
    try {
      $val = $func();
      parent::ok(1, $msg);
      return $val;
    }
    catch( Exception $e ) {
      parent::ok(0, $msg);
      return NULL;
    }
  }

  function line_to_text($level) {
    $bt = debug_backtrace();
    $filename = $bt[$level+1]["file"];
    if( array_key_exists($filename, $this->file_cache) ) {
      $file = $this->file_cache[$filename];
    }
    else {
      $file = $this->file_cache[$filename] = split("\n", file_get_contents($filename));
    }
    $func = $bt[$level+1]["function"];
    $lineno = $bt[$level+1]["line"] - 1;
    $line = $file[$lineno];
    while( !preg_match("/(?i:$func\s*\()/", $line) ) {
      $lineno--;
      $line = $file[$lineno] . "\n" . $line;
    }
    if( preg_match("/(?s:(?i:$func)\((.*)\);)/", $line, $match) ) {
      $msg = $match[1];
      $msg = trim(preg_replace("/\s+/s", " ", $msg));
    }

    if( array_key_exists($func, $this->OPS) ) {
      $op = $this->OPS[$func];
      $replacements = array();
      // first yank out all the balanced (), {}, [] and replace
      // with static string.  We need to get down to the basic arguments
      // to find the comma seperating the arguments in the source code.
      $count = 1;
      $marker = "#####GRP#####";
      while( $count ) {
        $msg = preg_replace_callback(
                   // array('/(\([^\(\)]*\))/s', '/(\{[^\{\}]*\})/s', '/(\[[^\[\]]*\])/s'),
                   '/(\([^\(\)]*\))/s',
                   function ($matches) use(&$replacements, $marker) {
                     array_unshift($replacements, $matches[1]);
                     return $marker;
                   },
                   $msg,
                   1,
                   $count
               );
      }
      
      // replace comma with operator, ie: "array(1,2,3), array(1,2,3)" => "array(1,2,3) == array(1,2,3)"
      $msg = preg_replace("/\s*,\s*/", " $op ", $msg);

      // put all the replacement code chunks back in reverse order they were parsed
      while( $replacements ) {
        $msg = substr_replace($msg, array_shift($replacements), strrpos($msg, $marker), strlen($marker));
      }

    }
    global $TestTrivialMaxCols;
    if( strlen($msg) > $TestTrivialMaxCols ) {
      return substr($msg, 0, $TestTrivialMaxCols)."...";
    }
    return $msg;
  }
}

global $t;
$t = new TestTrivial();
function PLAN()         { global $t; $t->level(1); $args = func_get_args(); return call_user_func_array(array($t,'PLAN'),$args); }
function OK()           { global $t; $t->level(1); $args = func_get_args(); return call_user_func_array(array($t,'OK'),$args); }
function NOK()          { global $t; $t->level(1); $args = func_get_args(); return call_user_func_array(array($t,'NOK'),$args); }
function IS()           { global $t; $t->level(1); $args = func_get_args(); return call_user_func_array(array($t,'IS'),$args); }
function LIKE()         { global $t; $t->level(1); $args = func_get_args(); return call_user_func_array(array($t,'LIKE'),$args); }
function THROWS()       { global $t; $t->level(1); $args = func_get_args(); return call_user_func_array(array($t,'THROWS'),$args); }
function NOTHROW()      { global $t; $t->level(1); $args = func_get_args(); return call_user_func_array(array($t,'NOTHROW'),$args); }
?>