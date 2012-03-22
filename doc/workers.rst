.. _workers:

*******************
Anatomy of a Worker
*******************

A :dfn:`worker` is a program that receives dispatches from ``mod_gearbox``. 
To create a worker, you must write:

* The core worker code. Gearbox provides its Worker API in C++. You can also write workers in PHP and Perl using Gearbox's `SWIG <http://swig.org/>`_ bindings. The worker code is responsible for things such as:

  * Defining handler methods for all incoming REST calls that Gearbox dispatches to the worker.
  * Performing some sort of work, such as building a system image based on the user's query).
  * Handing off work to child workers (if necessary).
  * Sending status, progress, and error updates back to Gearbox.

* Schemas that define the form of the JSON that the worker accepts. For example, your schemas require that any "get" request must include a particular JSON boolean flag. Gearbox enforces these schemas for you.
* Configuration files for :program:`apache` and Gearbox.
* A YICF defining the structure of the package, including some infrastructure for :program:`daemontools` in :file:`$ROOT/conf/gearbox/gearbox-handlers.d`.

This section provides examples of worker components using the worker 
`gearbox_barn_worker <http://dist.corp.yahoo.com/by-package/gearbox_barn_worker/>`_. 
This example worker happens to be written in PHP, but as mentioned above, you are 
more than welcome to write Perl and C++ workers as well.

.. _json_schemas:

Worker JSON Schemas
===================

Any incoming HTTP request to Gearbox, whether a ``GET``, ``PUT``, ``POST``, or ``DELETE``, 
may carry a JSON payload. You can define schemas for your workers that define what JSON 
is acceptable. If an incoming request does not match the JSON schema, Gearbox automatically
rejects the request for you, as demonstrated in :ref:`schema_validation`.

To understand how to define worker schemas, recall from :ref:`uri_layout` that a worker has one or more 
:dfn:`resources` which may contain zero or more :dfn:`objects`. For each resource your worker 
supports, you may define up to four schemas, one for each of the four Gearbox operation types. 

Gearbox Operations Types
------------------------

When Gearbox dispatches a request to your worker, that request carries an operation type.
There are four types of operations, but note that these operations do not exactly map 1-to-1 to 
the four HTTP methods:

* A ``get`` operation is the result of an HTTP ``GET``.
* A ``create`` operation is the result of an HTTP ``PUT`` or ``POST``.
* An ``update`` operation is the result of an HTTP ``PUT`` or ``POST``.
* A ``delete`` operation is the result of an HTTP ``DELETE``.

When a client invokes an HTTP ``PUT`` or ``POST``, Gearbox determines whether the web service 
request is creating a new object or updating an existing object. In addition to exposing the
operation type to you via the Worker API, Gearbox uses this information to validate the operation
against the appropriate schema. 

Each schema file is a JSON file of the form :file:`{operation}-{worker}-{resource}-{version}.js`.
For example, if your worker is named ``foo`` and supports two resources ``bar`` and ``baz``, you
would need to create these schema files and install them under file:`$ROOT/share/gearbox/schemas/`:

.. code-block:: none

    create-foo-bar-v1.js
    delete-foo-bar-v1.js
    get-foo-bar-v1.js
    update-foo-bar-v1.js
    create-foo-baz-v1.js
    delete-foo-baz-v1.js
    get-foo-baz-v1.js
    update-foo-baz-v1.js

Technically speaking, you are not required to create a schema file for every permutation of 
operation and resource. Gearbox only emits a warning if an expected schema is missing, not 
an error. However, there are two important reasons to provide schemas. 

First, schemas save you the trouble of having to validate incoming data yourself. There is 
no reason to write your own one-off validation code if Gearbox can take care of this 
tedious job for you. Second, schemas serve as a form of documentation about what input your
web service accepts. So even if your data validation needs are minimal or nonexistent, it 
is a good idea to provide a schema for every operation.

.. _json_schema:

JSON Schema Format Primer
-------------------------

Gearbox's schema format relies on `JSON Schema <http://tools.ietf.org/html/draft-zyp-json-schema-03>`_.
Note that the JSON Schema spec is still evolving and certain more esoteric concepts in the spec are 
poorly defined and therefore hard to implement consistently. The basics, however, work reasonably well. 

A JSON schema is itself a JSON object that defines acceptable values. For example: 

.. code-block:: javascript

    {
        "type": "null"
    }

is the pattern to use for an operation where the JSON payload must be ``null`` or nonexistent. This 
pattern is typical for DELETE operations, and reasonably common for GET operations.

For a slightly more complicated example, consider:

.. code-block:: javascript

    {
        "type": "object",
        "additionalProperties": false,
        "properties": {
            "name": {
                "type": "string"
            },
            "address": {
                "type": "string",
                "optional": true
            }
        }
    }

This JSON schema specifies that valid input must be a JSON object with a string ``name`` property and 
an optional string ``address`` property. This JSON object would pass validation:

.. code-block:: javascript

    {
        "name": "Jane Smith",
        "address": "123 Main Street, San Jose, CA"
    }

But this object would fail (required property ``name`` is missing):

.. code-block:: javascript

    {
        "address": "123 Main Street, San Jose, CA"
    }

As would this (``address`` is the wrong type):

.. code-block:: javascript

    {
        "name": "Joe Smith",
        "address": true
    }

As would this (additional properties beyond ``name`` and ``address`` are not permitted):

.. code-block:: javascript

    {
        "name": "Joe Smith",
        "phone_number": "408-555-1212"
    }

Some of the more relevant attributes of JSON Schema include:

* `type <http://tools.ietf.org/html/draft-zyp-json-schema-03#section-5.1>`_ -- Specifies the allowed data type: ``string``, ``number``, ``integer``, ``boolean``, ``object``, ``array``, ``null``, or ``any``.
* `properties <http://tools.ietf.org/html/draft-zyp-json-schema-03#section-5.2>`_ -- Defines the list of valid properties an object may have.
* `additionalProperties <http://tools.ietf.org/html/draft-zyp-json-schema-03#section-5.4>`_ -- Specifies whether an object may have additional freeform properties not defined by the ``properties`` attribute.
* `required <http://tools.ietf.org/html/draft-zyp-json-schema-03#section-5.7>`_ -- Indicates that the property must be present and have a value.
* `minimum <http://tools.ietf.org/html/draft-zyp-json-schema-03#section-5.9>`_ -- Defines the minimum value of a number property.
* `maximum <http://tools.ietf.org/html/draft-zyp-json-schema-03#section-5.10>`_ -- Defines the maximum value of a number property.
* `pattern <http://tools.ietf.org/html/draft-zyp-json-schema-03#section-5.16>`_ -- Provides a regular expression that the property value must match.

For more information, refer to the `draft specification for JSON Schema <http://tools.ietf.org/html/draft-zyp-json-schema-03>`_.

.. _worker_configuration_files:

Worker Configuration Files
==========================

At minimum, a worker requires two configuration files: an Apache configuration file 
and a Gearbox configuration file. 

.. _apache_configuration_file:

Apache Configuration File
-------------------------

Every worker must provide an Apache 2.x configuration file that registers your worker's 
base URI with Apache to be handled by ``mod_gearbox``. Your package should deploy the
file as :file:`$ROOT/conf/apache/{product}.conf`

At a minimum, the Apache configuration file should contain a ``<Location>`` directive 
specifying the location where the worker acts, along with directives specifying the 
filepath to the Gearbox configuration file and the handler to use for these files. 
A boilerplate Apache configuration file for a worker that acts on the URL ``/example`` 
would resemble:

.. code-block:: apache

    <Location /example>
        GearboxConfigFile $ROOT/conf/gearbox/example.conf
        SetHandler gearbox-handler
    </Location>

The list of relevant Apache directives includes:

* ``GearboxConfigFile`` -- Specifies the full filepath to the worker's 
  :ref:`Gearbox configuration file <gearbox_conf_file>`. The syntax is::
  
      GearboxConfigFile <filepath>
  
  This directive is **required**.
  
* ``GearboxSync`` -- Changes HTTP methods to act synchronously within Gearbox.
  As described in :ref:`rest_handler`, ``DELETE``, ``POST``, and ``PUT`` are
  asynchronous by default. However, you can change any of these methods
  to act synchronously by specifying the names of the HTTP methods in 
  ``GearboxSync``. The syntax is::
   
      GearboxSync <space separated list of methods>

  For example::
  
      GearboxSync PUT DELETE

  would change ``PUT`` and ``DELETE`` to be synchronous, but not ``POST``.

* ``SetEnv`` -- Sets an environment variable, which is then available to you via the 
  worker API. The syntax is::
  
      SetEnv <env-variable> <value>
      
*  ``SetHandler`` -- Forces all files in this location to be processed by a handler. 
   In this case, you must set the value to ``gearbox-handler`` so that Apache handles 
   your worker using Gearbox. The syntax is::
   
       SetHandler gearbox-handler
   
   This directive is **required**.
   
.. _gearbox_conf_file:

Gearbox Configuration File
--------------------------

Every worker must provide a Gearbox configuration file that defines the component
name and the daemons it is responsible for. Your package should deploy the file as
:file:`$ROOT/conf/gearbox/{product}.conf`.

The Gearbox configuration file format is JSON. The file supports a substitution
syntax of ``%{object.key}``, which gets expanded by the ``gearbox-svc-builder`` 
service during service initialization and in the worker itself.

For a substitution of ``%{foo.bar}``, the Gearbox service builder just 
substitutes in the value of the key ``bar`` in the JSON object ``foo``. 
``foo`` must be a top-level JSON object, not a string literal. Values
to be substituted must either be in the local configuration file, or 
one of the conf files in :file:`$ROOT/conf/gearbox/config.d/{filename}.conf`.
Your Gearbox configuration file automatically loads all configuration sections
from configuration files in :file:`$ROOT/conf/gearbox/config.d`, setting 
the section name to the basename of the file. For example, the value 
``%{gearbox.user}`` comes from the file 
:file:`$ROOT/conf/gearbox/config.d/gearbox.conf`, which includes a ``user`` key.

A boilerplate Gearbox configuration file for an example worker would resemble:

.. code-block:: javascript

    {
        "component" : "example",

        "daemons" : [{
            "name" : "worker",
            "logname": "%{component}",
            "command" : "$ROOT/bin/example $ROOT/conf/gearbox/example.conf",
            "count" : 1,
            "user" : "%{gearbox.user}"
        }]
    }

The list of fields includes: 

* ``component`` -- Specifies the job name that ``mod_gearbox`` dispatches to. A component 
  of ``"foobar"`` means that ``mod_gearbox`` dispatches requests to functions of the form 
  :samp:`do_{operation}_foobar_{resource}_{version}.`
  
  While it is theoretically possible to have one worker implement 
  :samp:`do_get_foobar_{resource}_{version}` and another implement 
  :samp:`do_post_foobar_{resource}_{version}`, this is not a good approach. Each
  worker should be solely responsible for handling requests for a single component. 
  If for some reason you have two workers that share the same component name and 
  register handler functions with the same name, Gearbox dispatches requests
  randomly between the two workers. 

* ``daemons`` -- Defines the list of different daemons associated with 
  your worker, as a JSON array of objects. Most workers only require one 
  type of daemon. The main ``gearbox_daemons`` package starts up multiple 
  daemons to handle basic Gearbox infrastructure. Advanced workers might 
  also need multiple types of daemons. For example, you might have a 
  "read" daemon that starts up ten copies using a ``count`` of 10, and 
  a "write" daemon that starts up only one copy.   

* ``name`` -- Provides an arbitrary name for your worker daemon for use in 
  :program:`daemontools` configuration. For example::
  
      $ sudo svstat /service/*
      /service/gearbox_daemons-delay
      /service/gearbox_daemons-gearmand
      /service/gearbox_daemons-worker-01
      ..
      /service/gearbox_daemons-worker-09

* ``logname`` -- A name used to identify your worker in the Gearbox logs. By 
  convention, this should be the same as your component name. You can use 
  the ``%{foo}`` substitution syntax to ensure that this is the case. 

* ``command`` -- The command to run for the worker, including any command line 
  parameters that the worker executable requires. Most Gearbox workers get 
  invoked by passing the full filepath to the configuration file as a command 
  line parameter::
  
      "command": $ROOT/bin/example $ROOT/conf/gearbox/example.conf

  This is a common pattern because the base ``Worker`` class's constructor
  takes the filepath of the configuration file as an argument.

* ``count`` -- The number of workers to spawn. Individual workers can only handle
  one request at a time. Workers are not threaded and do not fork by themselves, so
  if you need to handle multiple simultaneous requests, set the count greater 
  than one. This is conceptually similar to the way Apache pre-forks a process. 

* ``user`` -- The UNIX user to use when running the daemon. You can substitute
  the value ``%{gearbox.user}`` here to use the overall Gearbox UNIX user. 


Optional Configuration Files
----------------------------

Depending on the nature of your worker, you might need to provide additional
configuration files. For example, if you are writing a worker in PHP, and your 
worker needs permission to work with the filesystem, you might need to  
override ``open_basedir`` using an INI file:

.. code-block:: ini

   open_basedir = ${open_basedir}:$ROOT/var/gearbox/db/example/


Example Worker Code Walkthrough
===============================

This section walks through an example Gearbox worker, step by step. This 
particular worker is written in PHP, although you are free to write 
workers in other languages such as C++ and Perl. This is the same worker
we examined in :ref:`tutorial`. It is a good idea to walk through 
(or at least read) the tutorial before continuing with this section. 
You can
:download:`view the PHP source directly <examples/barn/bin/workerBarn.php>`.

Creating the Worker Class and Constructor
-----------------------------------------

.. highlight:: perl

.. Wait, why are we highlighting these PHP snippets as Perl? Because 
   Pygments is not smart enough to highlight PHP if there isn't a
   "<?php" present. Falling back to Perl syntax highlighting at least 
   kinda sorta works on isolated PHP code snippets. 

The first thing to do is to define our Worker class. In our model, each 
worker is actually an "animal" that lives in a "barn." Thus, we'll create
an ``Animal`` class that extends the base ``Worker`` class.

.. literalinclude:: examples/barn/bin/workerBarn.php
    :lines: 1-5

Before defining the class, we include the :file:`Gearbox.php` so that we 
have access to Gearbox's PHP Worker API.

We also define a constant ``DBDIR``, which defines the directory where
our worker will store persistent data. Storage is a worker implementation 
detail; you can use an RDBMS, a NoSQL data store, or anything you like. 
Some workers don't even need persistent storage at all. In our case, we'll 
implement a simple, brute force apporach where we store each worker "animal" 
as a JSON file in a subdirectory under ``$ROOT/var/gearbox/db/barn/``. 

Now we define the class and its constructor:

.. literalinclude:: examples/barn/bin/workerBarn.php
    :lines: 7-15

To successfully instantiate a worker, you must call the base ``Worker``
class's constructor, which requires the path to the worker's Gearbox 
configuration file. This is why you typically invoke workers using the
pattern described in :ref:`gearbox_conf_file`.

After calling the parent constructor, you can perform setup operations 
specific to the ``Animal`` class. The most important thing to do is to 
declare which worker methods are handlers meant to be invoked by Gearbox. 
Our worker supports ``GET``, ``PUT``, ``POST``, and ``DELETE`` HTTP calls, so 
we register four handler methods, following the naming convention defined
in :ref:`rest_api`.

Handling GET Requests
---------------------

The first handler method, :func:`do_get_barn_animal_v1`, handles two 
types of ``GET`` requests:

* Requests of the form :file:`/barn/v1/animal/{id}`, which retrieve information for a single animal
* Requests of the form :file:`/barn/v1/animal/`, which lists all animals currently in the barn

.. literalinclude:: examples/barn/bin/workerBarn.php
    :lines: 17-30

When Gearbox dispatches a request to a handler method, it provides a ``job`` 
object, which provides information about the job and the request, and a 
``resp`` object, which enables you to update the response. As mentioned in 
:ref:`rest_api`, arguments in a Gearbox request appear in the URI after the 
resource, as in :file:`/{component}/{version}/{resource}/{arg1}/{arg2}/...`.
Calling the :func:`arguments` method yields an array of the arguments 
supplied in the request. 

An empty arguments array means a request :file:`/barn/v1/animal`, so the 
response should be to list all animals in the barn. Each animal is just
a file under ``DBDIR/animals``, so we can use the PHP ``glob`` function
to retrieve an array of results. If the barn is empty, we make sure we 
return an empty array rather than ``false``. 

If the arguments array is not empty, the first argument must be the
ID of some animal. The :func:`get_animal` helper function is responsible 
for fetching an individual animal:

.. literalinclude:: examples/barn/bin/workerBarn.php
    :lines: 32-39

Since the filename of the animal is the ID, the function just fetches
the file's JSON contents. If the animal does not exist, we throw the 
Gearbox exception ``ERR_NOT_FOUND``. This automatically takes care of the 
response for us: it sets the code to 404 (like an HTTP 404 FILE NOT FOUND), 
the progress to 100, and the state to ``COMPLETED``.


Returning to :func:`do_get_barn_animal_v1`, the variable ``out`` could be 
an empty PHP array, a PHP array of animal names, or a PHP array representing 
an individual animal.

.. literalinclude:: examples/barn/bin/workerBarn.php
    :lines: 28-29

The function adds the JSON-encoded value of ``$out`` to  the response and 
returns ``WORKER::SUCCESS``. Like throwing an exception, returning 
``WORKER::SUCCESS`` causes Gearbox to set the response appropriately for us.
It updates the progress of the request to 100, sets the code to 0 
(indicating success), and moves the state to ``COMPLETED``. 


Handling PUT Requests
---------------------

The second handler method, :func:`do_put_barn_animal_v1`, handles a 
``PUT`` to :file:`/barn/v1/animal/{id}` by storing the literal contents
of that request under the given ID, then doing some "work," whatever 
we've defined that to be. If the animal already exists, the
method blows away the old contents and replaces it with the new contents.

.. literalinclude:: examples/barn/bin/workerBarn.php
    :lines: 41-52 

Thanks to our worker's innovative and sophisticated persistence mechanism 
of "saving a bunch of files in a directory," implementing this method is 
a simple matter of writing out the contents of the request to the right 
file name. If the ID is missing, the function throws an ``ERR_BAD_REQUEST`` 
exception, which causes Gearbox to set the response code to 400 
(like an HTTP 400 BAD REQUEST), the progress to 100, and the state to 
``COMPLETED``. Otherwise, once the file is written we run the mysterious 
:func:`do_work` method, and once that completes, return ``WORKER::SUCCESS``.
More about :func:`do_work` later.

Note that we do *not* have to check the contents of the file and verify 
that the contents are JSON and that the JSON content contains the fields
we need. Gearbox handles JSON validation for us automatically, thanks to
the JSON schemas defined for this worker. If the contents of the request
violated the schema, Gearbox rejects the request for us, and our worker
code never sees it. For more information, refer to :ref:`json_schema`.

Handling POST Requests
----------------------

The third handler method, :func:`do_post_barn_animal_v1`, handles two types
of  ``POST`` requests: 

* Requests of the form :file:`/barn/v1/animal/{id}`, which updates an existing animal
* Requests of the form :file:`/barn/v1/animal/`, which creates a new animal with an auto-generated ID

.. literalinclude:: examples/barn/bin/workerBarn.php
    :lines: 41-52

Gearbox automatically determines which case we're in by setting the
job operation type, which we can access by calling ``$job->operation()``. 

If the user posted to a URI without an ID, Gearbox identifies this as a 
"create" operation. The function creates a new animal. Note that
Gearbox automatically generates a unique ID for the new resource, which 
can be retrieved by calling ``$job->resource_name()``. If the animal does
not have its ``species`` field set, the function sets it to a default value. 

If the user provided a ID, Gearbox identifies this as an "update" operation.
In this case, the function assumes that the request is designed to modify 
the animal. Workers contain two fields: a required ``name`` field which 
cannot be changed after creation, and an optional ``species`` field. If 
you POST to an existing animal, the function updates the ``species`` field.

This is very different behavior than the ``PUT`` case. First, the user can
``POST`` directly to the ``/barn/v1/animal`` endpoint, with *no* ID provided, and 
this has the side effect of creating a new resource with an autogenerated ID. 
For ``PUT``, this would be an error. Second, the user can use POST to do a 
partial update of the animal's 
contents::

    {
        "species": "Some value"
    }

By contrast, ``PUT`` requests completely replace the contents of the animal.
There is no such thing as a partial update. This is an attempt to conform 
to REST conventions around ``PUT`` operations.

After creating or updating the animal, the method runs :func:`do_work`, 
and once that completes, returns ``WORKER::SUCCESS``. Note that the
user can legally just send an empty ``POST`` to an existing animal. This
results in kicking off some work without modifying the contents
of the animal at all. 

Of course, we don't have to combine "configuring the worker" and 
"starting work" all in the same method handler. There are many other 
valid patterns for handling incoming requests in Gearbox. For example, 
you could design a worker that uses ``PUT`` exclusively to set up and 
initially configure a worker resource, and ``POST`` exclusively to kick 
off some significant work. 

Handling DELETE Requests
------------------------

The final handler method, :func:`do_delete_barn_animal_v1`, handles a 
``DELETE`` to :file:`/barn/v1/animal/{id}` by unlinking the file that 
represents that animal.

.. literalinclude:: examples/barn/bin/workerBarn.php
    :lines: 78-94

As with ``PUT`` and ``POST``, our worker has Gearbox handle ``DELETE`` 
requests asynchronously. The user must poll a status object in order
to see the sad confirmation message, "Charlotte is now dead. Good job!"

Doing Work
----------

The :func:`do_work` and :func:`spin_web` methods, called from the ``PUT`` 
handler and the ``POST`` handler, are responsible for doing asynchronous work:

.. literalinclude:: examples/barn/bin/workerBarn.php
    :lines: 96-118

If the worker's species is "Araneus cavaticus" (a spider), we call the 
:func:`spin_web` helper method. Otherwise, we assume the worker is a pig,
in which case "work" consists of setting an allegedly humorous status message
and returning.

:func:`spin_web` attempts to simulate a long worker process, such as
building an OS image. In between calls to :func:`sleep()`, the method
updates the response's ``progress`` and ``message`` fields. If the user
polls the status object as described in :ref:`long_worker_jobs`, 
Gearbox returns the appropriate status messages and progress.
When the web is finished spinning, the method adds a final message 
and returns. The method does *not* return ``WORKER::SUCCESS``, instead
relying on the calling function to take care of that.

Of course, if you need to perform different types of work, you do not need to 
implement this by radically changing worker behavior based on a single 
configuration field. The implementation of how you divide worker code between
your actual classes is up to you.

Packaging the Worker
====================

Finally, let's walk 
:download:`through the YICF for packaging the worker <examples/barn/pkg/gearbox_barn_worker.yicf>`.
The YICF is useful because it describes in one place the infrastructure
files required to actually create a worker. 

.. literalinclude:: examples/barn/pkg/gearbox_barn_worker.yicf

The components in the package include:

01. :file:`conf/gearbox/gearbox-handlers.d/*` -- Infrastructure files required
    by Gearbox. Gearbox uses these filenames to check whether incoming HTTP 
    requests have incorrect URIs. You must always provide these files, one per
    handler method you support.
    
02. :file:`conf/gearbox/barn.conf`, :file:`conf/apache/barn.conf`, 
    and :file:`etc/php/gearbox_barn.ini` -- configuration files for Gearbox, apache, 
    and PHP respectively. You must always provide the first two types of configuration 
    files. The third file is only present because in PHP, we need a little extra 
    configuration to work with the filesystem. For more information, 
    refer to :ref:`worker_configuration_files`.
    
03. :file:`var/gearbox/db/barn/animals` -- an empty directory our particular package
    uses to persist data. If you use a database or have some other persistence 
    scheme, this kind of thing might be unnecessary. However, if you do use flat files, 
    the convention is to store them under :file:`var/gearbox/db/{your_product}/`. 
    
04. :file:`var/gearbox/services/gearbox_barn_worker` -- A directory for infrastructure 
    files required by :program:`daemontools`. You must always provide this directory.
    
05. :file:`bin/workerBarn.php` -- The actual executable to run when invoking the worker.
    You must always provide this directory. 

06. :file:`share/gearbox/schemas/` -- JSON schema files for your worker, as described
    in :ref:`json_schemas`. You do not have to provide schema files, but we strongly
    recommend you do so.
    
07. **package requirements** -- If you write a worker in C++, it might only need to depend 
    on ``gearbox_daemons``. Our example PHP worker also needs ``gearbox_php_lib``, plus
    some other PHP prerequisites. 

08. **start and stop scripts** -- On start, your worker should call 
    :file:`bin/gearbox-svc-builder-start {configfile}`. On stop, your worker should call 
    :file:`bin/gearbox-svc-builder-stop`. 
