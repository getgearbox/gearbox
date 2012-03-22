.. _rest_api:

****************
Gearbox REST API
****************

As mentioned in :ref:`overview`, Gearbox is a framework for building scalable, 
reliable, asynchronous REST services. This chapter describes how Gearbox 
routes HTTP requests for a given service.

.. _rest_handler:

REST Handler
============

The external interface to Gearbox software components are REST requests with 
JSON content, supported through the Apache 2.2 module ``mod_gearbox``. A 
REST call to a Gearbox service works like this:

01. A client makes an HTTP request (GET, HEAD, POST, PUT, or DELETE) to
    one of your server's URIs. Requests may carry a JSON payload.

02. Gearbox then validates the request by: 

    01. Validating that the request's base URI corresponds to a listener
        that that ``mod_gearbox`` has been configured to handle.

    02. Transforming the URI and HTTP method into a dispatchable
        worker method name and verifying that this method name has been 
        registered with Gearbox. The transformation is of the form:
        :samp:`do_{http_method}_{listener}_{resource}_{version}`. 
        For example, the HTTP request ``GET /cc/v1/network`` becomes 
        :func:`do_get_cc_network_v1`. For more detail about how this works,
        refer to :ref:`uri_layout`.
    
    03. Inspecting the JSON payload (if any) and validating it against a
        :ref:`JSON schema <json_schemas>` (if any). If the payload violates 
        the schema, ``mod_gearbox`` :ref:`rejects the request <schema_validation>`.
    
03. If the request passes validation, Gearbox dispatches the request
    to the worker method name. If the request is asynchronous, Gearbox
    also generates a status object in the database. 

04. Gearbox serializes the request and dispatches it as a job to the 
    open source `Gearman <http://gearman.org/>`_ job framework. 
    For synchronous jobs, ``mod_gearbox`` stays connected,
    waiting for the response. (See the function :cpp:func:`gearman_client_do` vs. 
    :cpp:func:`gearman_client_do_background` in the source code.)

05. Gearbox returns an HTTP response to client: either the response content 
    (synchronous) or the status object (asynchronous).

06. During all of these steps, Gearbox logs and handles any errors that occur.

By default, Gearbox treats ``GET`` and ``HEAD`` requests synchronously, and 
``PUT``, ``POST``, and ``DELETE`` requests asynchronously. API designers can 
change this behavior using an 
:ref:`Apache configuration file <apache_configuration_file>`.

For synchronous requests, Gearbox returns an HTTP status code 200 (OK) on 
success, along with a JSON response representing the resource requested. 
Synchronous requests tie up an Apache child and a Gearbox worker for 
as long as it takes to complete the request.

For asynchronous requests, Gearbox returns an HTTP status code 202 (Accepted),
along with a JSON object containing a URI that represents the status of the
request. Clients may poll this URI to determine whether the request has
succeeded, failed, or is still pending. Asynchronous requests tie up a
Gearbox worker, but the Apache child is free to service other requests once
it returns the initial 202 response.    

.. _uri_layout:

URI Layout
==========

For each valid incoming HTTP request, ``mod_gearbox`` maps the HTTP method
and request URI to a worker function. An incoming request like::

    GET /barn/v1/animal/Wilbur

causes Gearbox to invoke a function in the "barn" Worker class named::

    do_get_barn_animal_v1

passing in the string "Wilbur" as the first argument to that function.
More generally, the request::

    $method /$worker/$version/$resource/$arg1/$arg2/...

maps to the function call::

    do_$method_$worker_$resource_$version($job, $resp, ...)

where:

* ``$method`` is the lowercase name of one of the five HTTP methods 
  Gearbox supports: ``DELETE``, ``GET``, ``HEAD``, ``POST``, or ``PUT``.
  For more information, refer to :ref:`http_methods`.
  
* ``$worker`` is the name of the top level listener for this kind of 
  HTTP request. A worker is a C++, PHP, or Perl class that uses the Gearbox 
  Worker API, plus related resources. For more information about the 
  anatomy of a worker, refer to :ref:`workers`.
  
* ``$version`` is the version of the API. This string must be a lowercase 
  "v" followed by one or more digits. Typically this string is "v1".

* ``$resource`` is a target within a worker that you can get or operate on. 
  A resource usually represents a particular type of work that a worker 
  can perform. 
  
* Arguments are additional path elements in the HTTP request that get 
  passed into the function. To access these arguments in your worker code, call 
  the ``job`` parameter's ``arguments()`` method, which returns an array of arguments 
  in order. You can also supply data to worker functions by including a 
  JSON payload -- this is a typical pattern when doing a ``PUT`` or ``POST``. 
  To access the payload content in your worker code, call the ``job`` parameter's
  ``content()`` method. 

.. _http_methods:

Supporting HTTP Methods
=======================

Gearbox supports five HTTP methods: ``DELETE``, ``GET``, ``HEAD``, ``POST``,
and  ``PUT``. "Support" simply means that Gearbox is able to successfully 
route incoming HTTP requests that use one of these five methods. Once 
Gearbox hands off the request to a worker as described in :ref:`uri_layout`,
it is the worker method's responsibility to handle that request appropriately.
Workers do not need to support all five HTTP methods, and each method's 
implementation is left up to the worker.

This means that in theory, a worker can have any number of unorthodox
responses to these HTTP methods. For example, there is nothing stopping you 
from designing a worker that responds to a ``GET`` request by starting a new 
job. However, this is not recommended. In practice, you should follow these 
conventions:

* ``DELETE`` -- Use for removing a resource.

* ``GET`` -- Use for retrieving read-only information about a job. Do not
  use ``GET`` to modify a resource's fields or to control a job.

* ``HEAD`` -- Use for quickly checking information that can be inferred 
  from HTTP headers only. For example, you could check for the existence
  of a resource by making a ``HEAD`` call, which is more lightweight 
  then fetching the resource's headers *and* content. As with ``GET``,
  ``HEAD`` requests should be read-only.

* ``POST`` -- Use for job control, for creating resources, or for fully 
  or partially updating a resource's contents. Unlike ``PUT``, a ``POST``
  may perform partial updates and trigger side effects related to other
  resources.

* ``PUT`` -- Use for job control, for creating resources, or for fully
  updating a resource's contents. A ``PUT`` means that you must put 
  exactly the specified contents to exactly the specified URI. 
