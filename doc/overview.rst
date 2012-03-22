.. _overview:

****************
Gearbox Overview
****************

Gearbox is a framework for building scalable, reliable, asynchronous REST services in a standard manner.

Asynchronous Web Services
=========================

Gearbox is built to handle both synchronous and asynchronous web service requests. 
In a synchronous web service:

1. a user makes a HTTP request
2. the server does some work to construct a response
3. the server returns the response to you

During step 2, the thread handling the incoming request blocks. This is fine as long 
as the response returns quickly enough. In some use cases, a delay of several seconds
or even several tens of seconds might be acceptable. 

However, this system breaks down for web services that trigger computationally
expensive work, such as building an operating system image. A job that takes many 
minutes or hours to complete not only ties up a valuable server thread, but 
also threatens to run into network timeout issues and other instabilities. 

Asynchronous web services solve this problem by separating job execution from 
job control. In an asynchronous web service:

1. a user makes an HTTP request
2. the server queues up a job that will work on the task
3. the server returns a status object that references that queued job
4. a worker wakes up and completes the job out of the queue, updating the status along the way

In this model, the server returns a response to the user almost immediately, freeing 
up the web server thread and avoiding problems that can occur with long-lived
network connections. The work itself is done by a "worker" process that
runs outside the context of the web server, perhaps even on a different machine.

The main difference for end users is that the server response is a status object, 
indicating that the job is under way. The response also contains a URI that the user may 
poll to determine whether the job is done. As an alternative to polling, some
services might use a callback function, pinging an external URL to indicate that 
the job has completed.


What Does Gearbox Include?
==========================

So how does Gearbox actually help you build asynchronous REST web services? 

* Defining REST interfaces: Given an HTTP method and request URI, Gearbox 
  has a simple transformation convention for dispatching that request to a 
  previously registered worker method. You are responsible for implementing
  the worker method, but you do not need to worry about how to invoke your 
  method from the web serving layer.

* Validating input data: In addition to dispatching requests to worker methods,
  Gearbox also validates JSON payloads against schemas, rejecting illegal
  requests automatically. For more information, refer to :ref:`json_schemas`.

* Handling asynchronous requests: By default, Gearbox treats ``PUT``, 
  ``POST``, and ``DELETE`` as asynchronous requests. If an asynchronous 
  request comes in, Gearbox serves up and maintains a status object 
  representing the status of the request. 
  
* Managing the worker queue: Gearbox wraps the open source 
  `Gearman <http://gearman.org/>`_ job framework, invoking worker threads 
  in the foreground (synchronous) and background (asynchronous) on your 
  behalf. Gearbox is responsible for maintaining the worker queue. 
  The worker queue is persisted to 
  `sqlite <http://sqlite.org>`_, `mysql <http://mysql.com>`_, 
  or `Sherpa <http://devel.corp.yahoo.com/sherpa/guide/>`_.

* Providing a Worker API: The Gearbox API includes everything you need
  to handle incoming requests, return responses, set events and respond to
  events, and control other workers.  Although the primary language for 
  writing workers is C++, `SWIG <http://swig.org>`_ bindings are available 
  for writing workers in PHP or Perl.
