#!/usr/bin/env phprun
<?

require_once("Gearbox.php");
require_once("test-trivial.php");

PLAN(6);

# not much here, we really only import the consts
# for use with the JobManager::job(HttpClient::Method, Uri) api

IS( GearboxHttpClient::METHOD_GET, 0 );
IS( GearboxHttpClient::METHOD_DELETE, 1 );
IS( GearboxHttpClient::METHOD_POST, 2 );
IS( GearboxHttpClient::METHOD_PUT, 3 );
IS( GearboxHttpClient::METHOD_HEAD, 4 );
IS( GearboxHttpClient::METHOD_UNKNOWN, 5 );

?>
