.. _getting_started:

.. highlight:: none

***************
Getting Started
***************

Gearbox is a framework for building scalable, reliable, asynchronous REST services in 
a standard manner. This chapter is a tutorial for exercising an example Gearbox worker with 
the REST API. If you have never used Gearbox before, this chapter provides a concrete way 
to start understanding how Gearbox works. This chapter does *not* explain how to program 
against the Gearbox APIs. For that, please refer to :ref:`rest_api` and :ref:`workers`.

Before running through this tutorial, please read (or at least skim) :ref:`overview`
so that you have some understanding of the set of problems that the Gearbox framework 
is attempting to solve.

Installing and Starting Gearbox
===============================

To install Gearbox, you need merely create a clean environment to work in and install
the necessary packages.

    FIXME

.. _tutorial:

Tutorial: Exercising Gearbox
============================

Now that you have a working Gearbox installation, it is time to exercise 
the system. In this tutorial, you will install an example worker 
and run jobs through the REST API. 

Throughout this tutorial, we'll try to draw a strong distinction between
what the Gearbox framework is doing and what the worker code is doing. 
It is important to understand what functionality you get "for free" from 
Gearbox and what you are responsible for implementing yourself when
you write your worker.

Installing an Example Worker
----------------------------

First, you must install an example worker to exercise. This particular worker is 
written in PHP, so it drags in a few more dependencies. You may also write 
workers in C++ and Perl. 

01. Install the example worker package 
    FIXME

Simple GETs and PUTs
--------------------

As described in :ref:`uri_layout`, the structure of a Gearbox REST URI is 
composed of a *host*, *component*, *version*, *resource*, and optionally one or
more *arguments*. The first argument is referred to as the "resource name". 
Our example worker has a single component named ``barn`` 
and a single resource named ``animal``. 
 
01. Start by calling GET on the ``animal`` resource using :program:`curl`:: 
 
        $ curl -s http://localhost:4080/barn/v1/animal | json_reformat
    
    The :option:`-s` option suppresses progress meter and error messages. Piping
    :program:`curl`'s output to the :program:`json_reformat` utility just adds 
    newlines and spacing to make the web service response more human-readable. 
    :program:`json_reformat` is part of the ``yajl_y`` package, included with 
    ``gearbox_daemons``.
    
    The web service responds with a JSON object containing an empty array 
    named ``animals``:
    
    .. code-block:: javascript

        {
            "animals": [

            ]
        }
    
    So what just happened here?
    
    1. Gearbox took the incoming HTTP request, and based on the HTTP method and the 
    structure of the URL, dispatched the request to a worker function named 
    :cpp:func:`do_get_barn_animal_v1`. For more information about how Gearbox REST, 
    dispatching works, refer to :ref:`rest_handler`.
    
    2. The worker function determined what to do with your request and generated
    a JSON response. 
    
    3. Gearbox returned the web service response to you directly and *synchronously*. 
    Until the worker function returns with its response, the apache process handling
    this request is blocked. 
        
    Note that every worker web service has its own semantics. For
    ``gearbox_barn_worker``, this web service call is interpreted as a 
    "list all resource names" request -- and since the worker has no resource names yet, it
    returns an empty array. But this behavior is completely specific to 
    ``gearbox_barn_worker``! For similar HTTP requests, other workers might return 
    entirely different JSON responses, return no output, return a 4xx HTTP error code, etc.
     
02. Create a file called :file:`pig.json`. We will use this file to create a new animal
    in the farm. Populate the file with these contents:
 
    .. code-block:: javascript
    
        {
           "name": "Wilbur"
        }
    
    Then create a new resource name by PUTting this file to the web service::
 
        $ curl -s -X PUT -d@pig.json http://localhost:4080/barn/v1/animal/Wilbur | json_reformat
    
    The HTTP semantics of this web service call are, "Create the file at the specified URL, using
    the contents of 'pig.json'." The worker web service responds with another JSON object:
    
    .. code-block:: javascript
    
         {
           "children": [

           ],
           "ctime": 1297902089,
           "messages": [

           ],
           "mtime": 1297902089,
           "operation": "create",
           "progress": 0,
           "state": "PENDING",
           "status_uri": "http://localhost:4080/barn/v1/status/s-ft3yzt311xz52x4ahyccnp4ht7",
           "uri": "http://localhost:4080/barn/v1/animal/Wilbur"
         }
    
    Unlike the previous JSON response, this response is generated by Gearbox, not the worker. 
    This particular block of JSON is a Gearbox :dfn:`status response`, which indicates that 
    Gearbox handled the web service request asynchronously. You can learn more about the general 
    structure of a status response, by referring to :ref:`status_responses`. For now, let's just 
    focus on three of the fields:
    
    * ``state`` -- currently set to ``PENDING``, indicating that the PUT operation is not
      necessarily complete yet.
    * ``status_uri`` -- a URI provided by Gearbox that represents the status of this
      PUT request. You can poll this URI to determine how the operation is proceeding. 
    * ``uri`` -- the URI of the resource name you attempted to create.
    
    Another thing to note is that Gearbox automatically checked the validity of the data
    provided in the PUT request (the contents of :file:`pig.json`). For each operation,
    a worker may define a JSON schema. If the data in your request does not match the 
    requirements of the schema, Gearbox rejects the request immediately. For an example 
    of schema validation in action, refer to :ref:`schema_validation`. For more
    information about how to implement schemas, refer to :ref:`json_schemas`.
    
03. Check the PUT operation's progress by calling GET on the status URI::
  
        curl -s http://localhost:4080/barn/v1/status/s-ft3yzt311xz52x4ahyccnp4ht7 | json_reformat
    
    which results in another status response generated by Gearbox:

    .. code-block:: javascript
    
         {
           "children": [

           ],
           "code": 0,
           "ctime": 1297902089,
           "messages": [
             "Wilbur is happily rolling around in the mud!"
           ],
           "mtime": 1297902128,
           "operation": "create",
           "progress": 100,
           "state": "COMPLETED",
           "status_uri": "http://localhost:4080/barn/v1/status/s-ft3yzt311xz52x4ahyccnp4ht7",
           "uri": "http://localhost:4080/barn/v1/animal/Wilbur"
         }
    
    By the time we manage to type in a new :program:`curl` request, the ``state`` has already 
    changed to ``COMPLETED`` and the ``progress`` field has updated from 0 to 100. This 
    indicates that as far as Gearbox is concerned, the PUT operation completed successfully. 
    Our pig Wilbur has been created, and he resides at http://localhost:4080/barn/v1/animal/Wilbur.
    (We'll check this shortly.)

    In addition, the string "Wilbur is happily rolling around in the mud!" appears in the message
    queue. Workers can update the ``progress`` field and add arbitrary messages, but the rest 
    of the response is constructed by Gearbox.
    
04. Check Wilbur himself by calling GET on his URI::
 
        $ curl -s http://localhost:4080/barn/v1/animal/Wilbur | json_reformat
    
    and we are relieved to discover that Wilbur's data is exactly what we PUT to that URI:
    
    .. code-block:: javascript
    
         {
           "name": "Wilbur"
         }
    
You might be asking yourself, "Why this complicated two-step around polling status 
responses? Why not allow me to PUT data directly, and respond immediately with success or
failure?" In fact, many real-world web services work this way. However, Gearbox is 
built around the assumption that update operations such as PUT, POST, and DELETE might
fire off processes that take a long, unpredictable time to complete. Therefore, by 
default Gearbox handles these kinds of requests :dfn:`asynchronously`. Instead of 
blocking other work until the job completes, Gearbox hands the work off to a worker
process and returns immediately with a status response, which you can poll for progress.

.. note:: Although PUT, POST, and DELETE are asynchronous by default, you can configure
          HTTP methods to be synchronous or asynchronous on a per-worker basis. For more
          information, refer to :ref:`apache_configuration_file`.

It turns out that in ``gearbox_barn-worker``, a PUT operation for a pig is very fast 
because pigs don't do any actual useful work -- they just roll around in the mud.
So here, the asynchronous model here is almost certainly overkill. But later in the tutorial, 
we'll see an example of a worker that does something more substantial -- a use case where the 
asynchronous model makes more sense. 

The Difference Between PUT and POST
-----------------------------------

When designing a worker, it is completely up to you which HTTP methods to support. For
pedagogical purposes, ``gearbox_barn_worker`` supports POST as well us PUT. Both 
operations can be used to create resource names, but the two operations have different semantics.

01. Create a file called :file:`pig2.json` and give it these contents:
 
    .. code-block:: javascript
    
        {
           "name": "Hammy McHammerson"
        }

02. Use :program:`curl` to POST the contents of this file to the ``/barn/v1/animal`` endpoint::
   
        $ curl -s -X POST -d@pig2.json http://localhost:4080/barn/v1/animal | json_reformat

    This generates a new status response:
    
    .. code-block:: javascript
    
         {
           "children": [

           ],
           "ctime": 1297902241,
           "messages": [

           ],
           "mtime": 1297902241,
           "operation": "create",
           "progress": 0,
           "state": "PENDING",
           "status_uri": "http://localhost:4080/barn/v1/status/s-dm9pfyhekakwa9bj00mepnnkea",
           "uri": "http://localhost:4080/barn/v1/animal/a-6xh9p61rczmddwett5ahzwmke1"
         }

    There's already a key difference between this POST operation and the previous PUT
    operation. We posted to the ``animal`` resource, ``/barn/v1/animal``, not 
    ``/barn/v1/animal/Wilbur``. And in the status response, the ``uri`` field ends with 
    a Gearbox-generated resource ID, ``a-6xh9p61rczmddwett5ahzwmke1``. Unlike the previous 
    case, we didn't get to choose our resource ID -- the system made one for us. This outlines
    a subtle difference between POSTs and PUTs. 
    
    * According to REST conventions, a PUT URI always contains a resource ID, as in 
      :file:`/{component}/{version}/{resource}/{id}`. The resource content is created outside
      the REST API itself (such as by authoring a file named "pig.json"). If you GET the same URI, 
      the response should contain exactly the same content that you PUT there earlier.
    * A POST URI is for the case where the worker job for creating a resource is responsible
      for populating part of the resource. In this case, the POST URI would lack a resource ID,
      as in :file:`/{component}/{version}/{resource}`, and Gearbox generates a globally unique
      ID for the new resource name. If you GET this resource name, the response is allowed to
      be a superset of the input from the post.

03. Check the POST operation's progress by calling GET on the status URI::
 
        $ curl -s http://localhost:4080/barn/v1/status/s-dm9pfyhekakwa9bj00mepnnkea | json_reformat

    The results are similar to the PUT case, albeit with different names and URIs:
    
    .. code-block:: javascript
    
         {
           "children": [

           ],
           "code": 0,
           "ctime": 1297902241,
           "messages": [
             "Hammy McHammerson is happily rolling around in the mud!"
           ],
           "mtime": 1297902291,
           "operation": "create",
           "progress": 100,
           "state": "COMPLETED",
           "status_uri": "http://localhost:4080/barn/v1/status/s-dm9pfyhekakwa9bj00mepnnkea",
           "uri": "http://localhost:4080/barn/v1/animal/a-6xh9p61rczmddwett5ahzwmke1"
         }
    
     Once the progress reaches 100, a ``code`` field appears. A "0" value for code is always 
     successful. If there had been an error, the code would be a non-zero number, such as 404 
     if the resource was not found.

     The state is now ``COMPLETED``. This will be true even if the worker function failed. 
     ``COMPLETED`` means "done," not necessarily "done successfully."

04. As a sanity check, GET the contents of the second pig::
 
        $ curl -s http://localhost:4080/barn/v1/animal/a-6xh9p61rczmddwett5ahzwmke1 | json_reformat

    Surprise! The contents are *not* exactly the same as what we included in the POSTed file. 

    .. code-block:: javascript

         {
           "name": "Hammy McHammerson",
           "species": "Sus domestica"
         }
    
    Where did this extra ``species`` field come from? It turns out that ``gearbox_barn_worker`` has 
    specific logic around POST operations: if a new resource name is created and the ``species`` field is 
    missing, the worker assumes the resource name is a pig and helpfully sets the species accordingly. 
    This kind of thing is fine to do in POST operations, because POST operations may have side effects. 

05. If you now list the contents of the barn::
 
        $ curl -s http://localhost:4080/barn/v1/animal | json_reformat 

    the response indicates that the barn contains two pigs, Wilbur and Hammy McHammerson:

    .. code-block:: javascript

         {
           "animals": [
             "Wilbur",
             "a-6xh9p61rczmddwett5ahzwmke1"
           ]
         }

.. _schema_validation:

Schema Validation
-----------------

As mentioned earlier, workers may define :dfn:`schemas` for each operation that define the structure
of the input JSON that is acceptable. Gearbox is responsible for enforcing these schemas. 
Let's see this in action.

01. Create a file called :file:`rat.json` and give it these contents:

    .. code-block:: javascript
   
       {
          "name": "Templeton",
          "species": "Rattus norvegicus",
          "personality": "greedy"
       }

02. Use :program:`curl` to POST the contents of this file to the ``/barn/v1/animal`` endpoint::

        $ curl -s -X POST -d@rat.json http://localhost:4080/barn/v1/animal | json_reformat

    The status response is:
 
    .. code-block:: javascript
 
         {
           "code": 400,
           "messages": [
             "Json Exception: invalid property \"personality\": schema does not allow for this property"
           ],
           "operation": "update",
           "progress": 100,
           "state": "COMPLETED",
           "uri": "http://localhost:4080/barn/v1/animal"
         }

    The operation returns with no ``status_uri`` and an Exception message generated by Gearbox. 
    It turns out ``gearbox_barn_worker``'s schemas do not allow arbitrary new fields such as 
    ``personality``. Because the request violates the schema, ``mod_gearbox`` rejects the request 
    without bothering to dispatch anything to the worker. Needless to say, the rat "Templeton" is 
    not created.
    

.. _long_worker_jobs:

Long-lived Worker Jobs 
----------------------

Gearbox is explicitly designed to support long, complex jobs. Let's take a look at a slightly 
more interesting example, where our worker performs a job that takes a more substantial amount 
of time.

01. Create a file called :file:`spider.json` and give it these contents:

    .. code-block:: javascript
  
         {
            "name": "Charlotte",
            "species": "Araneus cavaticus"
         }

02. Use :program:`curl` to PUT the contents of this file into Gearbox::

        $ curl -s -X PUT -d@spider.json http://localhost:4080/barn/v1/animal/Charlotte | json_reformat

    Gearbox responds:
    
    .. code-block:: javascript
    
         {
           "children": [

           ],
           "ctime": 1297902508,
           "messages": [

           ],
           "mtime": 1297902508,
           "operation": "create",
           "progress": 0,
           "state": "PENDING",
           "status_uri": "http://localhost:4080/barn/v1/status/s-10anhq8dtc0mk08hbxescwqk8d",
           "uri": "http://localhost:4080/barn/v1/animal/Charlotte"
         }
    
    The results are similar to what you saw when you created Wilbur. However, if you immediately check the status URI::

        $ curl -s http://localhost:4080/barn/v1/status/s-10anhq8dtc0mk08hbxescwqk8d | json_reformat
    
    the web service returns a status response that looks different:
    
    .. code-block:: javascript

         {
           "children": [

           ],
           "ctime": 1297902508,
           "messages": [
             "Charlotte is beginning to spin a web."
           ],
           "mtime": 1297902524,
           "operation": "create",
           "progress": 10,
           "state": "RUNNING",
           "status_uri": "http://localhost:4080/barn/v1/status/s-10anhq8dtc0mk08hbxescwqk8d",
           "uri": "http://localhost:4080/barn/v1/animal/Charlotte"
         }
    
    Note the different message, the different state (``RUNNING`` instead of ``COMPLETED``), 
    and different progress (10 instead of 100). Unlike the previous example for Wilbur, the 
    job has *not* completed immediately. Also note that the status ``mtime`` has changed to 
    the timestamp that the progress and messages were updated. `mtime`` always corresponds to 
    the timestamp of the last update. If a job is in the ``COMPLETED`` state, ``mtime`` indicates
    when the job finished.
    
    The reason the behavior changed is that setting ``species`` to "Araneus cavaticus" triggers a 
    different code path in ``gearbox_barn_worker``. It is the worker's responsibility to process the 
    input payload, and in this case, the worker is designed to start a much longer "spider" task 
    ("spin a web") than the default "pig" task ("roll in the mud").

03. Monitor the job by polling the status periodically. Thirty seconds after launching the job,
    checking the status URI again results in:
    
    .. code-block:: javascript
    
         {
           "children": [

           ],
           "ctime": 1297902508,
           "messages": [
             "Charlotte is beginning to spin a web.",
             "Charlotte is still spinning..."
           ],
           "mtime": 1297902557,
           "operation": "create",
           "progress": 50,
           "state": "RUNNING",
           "status_uri": "http://localhost:4080/barn/v1/status/s-10anhq8dtc0mk08hbxescwqk8d",
           "uri": "http://localhost:4080/barn/v1/animal/Charlotte"
         }
    
    The worker has added a new message to the messages array, and updated the progress from 10 to 50. 
    Checking again thirty seconds after that results in:
    
    .. code-block:: javascript
    
         {
           "children": [

           ],
           "code": 0,
           "ctime": 1297902508,
           "messages": [
             "Charlotte is beginning to spin a web.",
             "Charlotte is still spinning...",
             "Charlotte's web is complete! It says: 'RADIANT'"
           ],
           "mtime": 1297902579,
           "operation": "create",
           "progress": 100,
           "state": "COMPLETED",
           "status_uri": "http://localhost:4080/barn/v1/status/s-10anhq8dtc0mk08hbxescwqk8d",
           "uri": "http://localhost:4080/barn/v1/animal/Charlotte"
         }
    
    The job has completed successfully. The key idea here is that "spinning a web" could 
    be some long, resource-intensive task such as constructing a virtual machine or 
    deploying an image or building a package. Since Gearbox handles these tasks asynchronously,
    you can fire off lots of long-running tasks in parallel without blocking other incoming 
    requests or trying to hold on to HTTP connections. 

04. At any point, you can check the Charlotte resource name directly, just as you did for Wilbur::
    
        $ curl -s http://localhost:4080/barn/v1/animal/Charlotte | json_reformat

    which returns the expected:
    
    .. code-block:: javascript
    
         {
           "name": "Charlotte",
           "species": "Araneus cavaticus"
         }

    You can also list all resource names::

        $ curl -s http://localhost:4080/barn/v1/animal | json_reformat
    
    and the web service responds with three animals in the barn:
    
    .. code-block:: javascript
    
         {
           "animals": [
             "Charlotte",
             "Wilbur",
             "a-6xh9p61rczmddwett5ahzwmke1"
           ]
         }

05. What happens if you POST to Charlotte? ::

        $ curl -s -X POST http://localhost:4080/barn/v1/animal/Charlotte | json_reformat
    
    This request is an empty POST (there is no :samp:`-d@{filename}`). It has no effect on 
    Charlotte's contents, but it does return another status response:
    
    .. code-block:: javascript
    
         {
           "children": [

           ],
           "ctime": 1297902641,
           "messages": [

           ],
           "mtime": 1297902641,
           "operation": "update",
           "progress": 0,
           "state": "PENDING",
           "status_uri": "http://localhost:4080/barn/v1/status/s-4p6gy4khcnfn5ad7h4grc20ge5",
           "uri": "http://localhost:4080/barn/v1/animal/Charlotte"
         }
    
    which, if we GET the new status URI, turns out to be another web spinning job:

    .. code-block:: javascript
    
         {
           "children": [

           ],
           "ctime": 1297902641,
           "messages": [
             "Charlotte is beginning to spin a web."
           ],
           "mtime": 1297902674,
           "operation": "update",
           "progress": 10,
           "state": "RUNNING",
           "status_uri": "http://localhost:4080/barn/v1/status/s-4p6gy4khcnfn5ad7h4grc20ge5",
           "uri": "http://localhost:4080/barn/v1/animal/Charlotte"
         }
         
    eventually resulting in another completed web: 

    .. code-block:: javascript

        {
          "children": [

          ],
          "code": 0,
          "ctime": 1297902641,
          "messages": [
            "Charlotte is beginning to spin a web.",
            "Charlotte is still spinning...",
            "Charlotte's web is complete! It says: 'HUMBLE'"
          ],
          "mtime": 1297902704,
          "operation": "update",
          "progress": 100,
          "state": "COMPLETED",
          "status_uri": "http://localhost:4080/barn/v1/status/s-4p6gy4khcnfn5ad7h4grc20ge5",
          "uri": "http://localhost:4080/barn/v1/animal/Charlotte"
        }

    Comparing the two jobs, notice that the ``operation`` field is set to ``update`` 
    rather than ``create``. For PUTs, the operation is always ``create``. For POSTs,
    the operation is ``create`` if no resource ID is specified in the URI, ``update``
    otherwise. The API makes this information available to the worker. 
        
    Again, what the worker does with a create vs. an update is worker implementation detail. 
    Our example worker ``gearbox_barn_worker`` is designed to use a "update" request to trigger 
    more work. But a worker could handle an update request by changing its behavior, starting an 
    entirely different kind of task, or even ignoring the request completely. 

06. In addition to creating resource names, you can also destroy them. Use 
    :program:`curl` to generate a DELETE request::
 
        $ curl -s -X DELETE http://localhost:4080/barn/v1/animal/Charlotte

    As with PUT and POST, Gearbox treats DELETE asynchronously by default. Thus, the 
    DELETE request returns a status response:
    
    .. code-block:: javascript
    
         {
           "children": [

           ],
           "ctime": 1297902794,
           "messages": [

           ],
           "mtime": 1297902794,
           "operation": "delete",
           "progress": 0,
           "state": "PENDING",
           "status_uri": "http://localhost:4080/barn/v1/status/s-7csbg2ee4mn4xtm69q85vk1gj5",
           "uri": "http://localhost:4080/barn/v1/animal/Charlotte"
         }

    As with ``create`` and ``update``, ``delete`` is another operation that Gearbox 
    recognizes and makes available to the worker. 
    
    Subsequently checking the status URI reveals that Charlotte has indeed been successfully deleted:
    
    .. code-block:: javascript
        
         {
           "children": [

           ],
           "code": 0,
           "ctime": 1297902777,
           "messages": [
             "Charlotte is now dead. Good job!"
           ],
           "mtime": 1297902849,
           "operation": "delete",
           "progress": 100,
           "state": "COMPLETED",
           "status_uri": "http://localhost:4080/barn/v1/status/s-7csbg2ee4mn4xtm69q85vk1gj5",
           "uri": "http://localhost:4080/barn/v1/animal/Charlotte"
         }
    
    For good measure, you can check Charlotte's URI directly::
    
        $ curl -s http://localhost:4080/barn/v1/animal/Charlotte | json_reformat
    
    which now returns a 404 error:
    
    .. code-block:: javascript
    
         {
           "code": 404,
           "messages": [
             "NOT_FOUND [404]: ERR_NOT_FOUND: Sorry, the barn does not contain Charlotte."
           ],
           "operation": "get",
           "progress": 100,
           "state": "COMPLETED",
           "uri": "http://localhost:4080/barn/v1/animal/Charlotte"
         }

    As with GET, PUT, and POST, the exact behavior for DELETE is a worker implementation
    detail. It is possible to design workers that leave DELETE handling unimplemented, 
    though this is little consolation for friends of Charlotte.
