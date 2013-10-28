#!/usr/bin/env python

import json
import os
import sys
import time

from gearbox import *


def file_get_contents(filename):
    with open(filename, 'r') as f:
        contents = f.read()
    return contents


def file_put_contents(filename, contents):
    with open(filename, 'wb') as f:
        f.write(contents)


class WorkerTestChainedPython(Worker):
    DBDIR = "/var/gearbox/db/test-chained-python/"

    def __init__(self, config):
        super(WorkerTestChainedPython, self).__init__(config)

        self.register_handler("do_get_testchainedpython_hello_v1")
        self.register_handler("do_get_internalpython_hello1_v1")
        self.register_handler("do_post_testchainedpython_hello2_v1")

        self.register_handler("do_get_testchainedpython_goodbye_v1")
        self.register_handler("do_post_testchainedpython_goodbye_v1")
        self.register_handler("do_append_internalpython_goodbye1_v1")
        self.register_handler("do_append_internalpython_goodbye2_v1")

        self.register_handler("do_get_testchainedpython_thing_v1")
        self.register_handler("do_post_testchainedpython_thing_v1")
        self.register_handler("do_reg_internalpython_service1_v1")
        self.register_handler("do_post_testchainedpython_service2_v1")

        self.register_handler("do_delete_testchainedpython_thing_v1")
        self.register_handler("do_unreg_internalpython_service1_v1")
        self.register_handler("do_delete_testchainedpython_service2_v1")

    # I am not sure why we would want to do a chained syncronous get, but
    # you can chain a bunch of sync jobs together
    def do_get_testchainedpython_hello_v1(self, job, resp):
        content = json.loads('{"message": "Hello from job"}')

        # do internal hello1 which just appends it name to our content
        j = Job(job)
        j.name("do_get_internalpython_hello1_v1")
        j.type(Job.JOB_SYNC)
        j.content(json.dumps(content))
        r = j.run()

        #check status
        r_status = r.status()
        if not r_status.is_success():
            err_code_class = "ERR_CODE_%s" % r_status.code()
            exception_to_raise = globals()[err_code_class]
            messages = r_status.messages()
            raise exception_to_raise(messages[0])

        # create sync http rest job back to localhost which takes
        # the output from previous job and adds its own name.
        j = self.job_manager().job(HttpClient.METHOD_POST,
                                   job.base_uri() + "/hello2")
        j.content(r.content())
        j.headers(r.headers())
        r = j.run()

        #check status
        r2_status = r.status()
        if not r2_status.is_success():
            err_code_class = "ERR_CODE_%s" % r2_status.code()
            exception_to_raise = globals()[err_code_class]
            messages = r2_status.messages()
            raise exception_to_raise(messages[0])

        output = json.loads(r.content())
        resp.content(json.dumps(output['message']))
        resp.headers(r.headers())
        return Worker.WORKER_SUCCESS

    def do_get_internalpython_hello1_v1(self, job, resp):
        job_content = json.loads(job.content())
        job_content['message'] += " and job1"

        resp.add_header("job1-header", "1")
        resp.content(json.dumps(job_content))
        return Worker.WORKER_SUCCESS

    # self is a SYNC post call configured via the httpd-test-chained.conf
    def do_post_testchainedpython_hello2_v1(self, job, resp):
        job_content = json.loads(job.content())
        job_content['message'] += " and job2"
        resp.headers(job.headers())
        resp.add_header("job2-header", "1")
        resp.content(json.dumps(job_content))
        return Worker.WORKER_SUCCESS

    def do_get_testchainedpython_goodbye_v1(self, job, resp):
        resource_file = os.path.join(self.DBDIR, job.resource_name())
        resp.content(file_get_contents(resource_file))
        return Worker.WORKER_SUCCESS

    def do_post_testchainedpython_goodbye_v1(self, job, resp):
        resp.status().add_message("processing from %s" % job.name())
        content = '{"message": "Goodbye from job"}'

        resource_file = os.path.join(self.DBDIR, job.resource_name())
        file_put_contents(resource_file, content)

        # do internal goodbye1 which just appends its name to our content
        self.afterwards(job, "do_append_internalpython_goodbye1_v1")
        # don't finalize the status, are going to keep going
        return Worker.WORKER_CONTINUE

    def do_append_internalpython_goodbye1_v1(self, job, resp):
        resp.status().add_message("processing from %s" % job.name())
        resource_file = os.path.join(self.DBDIR, job.resource_name())
        content = json.loads(file_get_contents(resource_file))
        content['message'] += " and job1"
        file_put_contents(resource_file, json.dumps(content))

        # do internal goodbye2 which just appends it name to our content
        self.afterwards(job, "do_append_internalpython_goodbye2_v1")
        # don't finalize the status, are going to keep going
        return Worker.WORKER_CONTINUE

    def do_append_internalpython_goodbye2_v1(self, job, resp):
        resp.status().add_message("processing from %s" % job.name())
        resource_file = os.path.join(self.DBDIR, job.resource_name())
        content = json.loads(file_get_contents(resource_file))
        content['message'] += " and job2"

        file_put_contents(resource_file, json.dumps(content['message']))

        # finally done so dont continue
        return Worker.WORKER_SUCCESS

    def do_get_testchainedpython_thing_v1(self, job, resp):
        resource_file = os.path.join(self.DBDIR, job.resource_name())
        resp.content(file_get_contents(resource_file))
        return Worker.WORKER_SUCCESS

    def do_post_testchainedpython_thing_v1(self, job, resp):
        resp.status().add_message("processing from %s" % job.name())
        out = {}
        out["id"] = job.resource_name()
        resource_file = os.path.join(self.DBDIR, job.resource_name())
        file_put_contents(resource_file, json.dumps(out))

        # our new thing needs to be registered with 2 fancy
        # services. They can both be registered at the same
        # time in parallel.
        jm = self.job_manager()

        responses = []

        # service 1 is registered via async local worker
        service1_job = jm.job("do_reg_internalpython_service1_v1")
        content = json.dumps(out)
        service1_job.content(content)
        responses.append(service1_job.run())

        service2_job = jm.job(HttpClient.METHOD_POST,
                              job.base_uri() + "/service2")
        service2_job.content(content)
        responses.append(service2_job.run())

        while responses:
            s = responses[0].status()
            s.sync()

            if s.has_completed():
                if s.is_success():
                    responses.pop()
                else:
                    err_class = "ERR_CODE_%s" % s.code()
                    msgs = s.messages()
                    exception_to_raise = globals()[err_class]
                    raise exception_to_raise(msgs)

            # pause between polling again
            time.sleep(1)

        return Worker.WORKER_SUCCESS

    def do_reg_internalpython_service1_v1(self, job, resp):
        resp.status().add_message("service1 registered")
        return Worker.WORKER_SUCCESS

    def do_post_testchainedpython_service2_v1(self, job, resp):
        resp.status().add_message("service2 registered")
        return Worker.WORKER_SUCCESS

    def do_delete_testchainedpython_thing_v1(self, job, resp):
        # our new thing needs to be unregistered with 2 fancy
        # services.  service 1 must be unregistered before service 2

        resource_file = os.path.join(self.DBDIR, job.resource_name())
        content = file_get_contents(resource_file)

        jm = self.job_manager()

        jobs = [list(), list()]

        # first gen jobs only has service1, unregister happens via local worker
        # put in first list
        jobs[0].append(jm.job("do_unreg_internalpython_service1_v1"))

        # second gen jobs only has service 2, unregister happens via DELETE
        # http call on remote worker
        #put in second list
        jobs[1].append(jm.job(HttpClient.METHOD_DELETE,
                              job.base_uri() + "/service2"))

        jm.job_queue_apply(jobs, "content", content)

        jm.job_queue_run(jobs)
        return Worker.WORKER_SUCCESS

    def do_unreg_internalpython_service1_v1(self, job, resp):
        resp.status().add_message("service1 unregistered")
        return Worker.WORKER_SUCCESS

    def do_delete_testchainedpython_service2_v1(self, job, resp):
        resp.status().add_message("service2 unregistered")
        return Worker.WORKER_SUCCESS

if __name__ == "__main__":
    #import pydevd
    #pydevd.settrace('192.168.86.1', port=62689, stdoutToServer=True,
    #                stderrToServer=True)

    worker = WorkerTestChainedPython(sys.argv[2])
    worker.run()
