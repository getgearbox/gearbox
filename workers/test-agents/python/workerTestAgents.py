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
    with open(filename, 'w') as f:
        f.write(contents)


class WorkerTestAgentsPython(Worker):
    DBDIR = "/var/gearbox/db/test-agents-python/"

    def __init__(self, cfg):
        super(WorkerTestAgentsPython, self).__init__(cfg)

        self.register_handler("do_get_testagentspython_thing_v1",
                              self.thing_handler)
        self.register_handler("do_post_testagentspython_thing_v1",
                              self.thing_handler)
        self.register_handler("do_delete_testagentspython_thing_v1",
                              self.thing_handler)
        self.register_handler("do_reg_testagentspython_A_v1",
                              self.dummy_handler)
        self.register_handler("do_unreg_testagentspython_A_v1",
                              self.dummy_handler)
        self.register_handler("do_reg_testagentspython_B_v1",
                              self.dummy_handler)
        self.register_handler("do_unreg_testagentspython_B_v1",
                              self.dummy_handler)
        self.register_handler("do_reg_testagentspython_C_v1",
                              self.dummy_handler)
        self.register_handler("do_unreg_testagentspython_C_v1",
                              self.dummy_handler)
        self.register_handler("do_reg_testagentspython_D_v1",
                              self.dummy_handler)
        self.register_handler("do_unreg_testagentspython_D_v1",
                              self.dummy_handler)

    def thing_handler(self, job, resp):
        resource_file = os.path.join(self.DBDIR, job.resource_name())
        content = {}
        if os.path.exists(resource_file):
            content = json.loads(file_get_contents(resource_file))

        if job.operation() == "get":
            resp.content(json.dumps(content))
            return Worker.WORKER_SUCCESS

        py_agents_conf = "/etc/gearbox/test-agents-python-agents.conf"
        agents = json.loads(file_get_contents(py_agents_conf))

        resp.status().add_message("calling agents")

        if job.operation() == "create":
            content['id'] = job.resource_name()
            file_put_contents(resource_file, json.dumps(content))

            jm = self.job_manager()

            run_agents_job = jm.job("do_run_global_agents_v1")
            agents_content = dict()
            agents_content['agents'] = agents['register']
            agents_content['content'] = json.dumps(content)
            run_agents_job.content(json.dumps(agents_content))

            run_agents = run_agents_job.run()
            run_status = run_agents.status()
            # poll for agents to be done
            while True:
                time.sleep(1)
                run_status.sync()
                if run_status.has_completed():
                    break

            if not run_status.is_success():
                err_code_class = "ERR_CODE_%s" % run_status.code()
                exception_to_raise = globals()[err_code_class]
                messages = run_status.messages()
                raise exception_to_raise(messages[0])
        else:
            # operation == delete
            job_manager = self.job_manager()
            queue = job_manager.job_queue(agents['unregister'])
            job_manager.job_queue_apply(queue, "content", json.dumps(content))
            job_manager.job_queue_run(queue)
            os.unlink(resource_file)
        resp.status().add_message("done")
        return Worker.WORKER_SUCCESS

    def dummy_handler(self, job, resp):
        contents = json.loads(job.content())
        resource_type = job.resource_type()

        if job.operation() == "reg":
            operation = "registered"
        else:
            operation = "unregistered"

        content_id = contents['id']

        msg = "%(resource_type)s %(operation)s for %(content_id)s" % locals()
        resp.status().add_message(str(msg))

        # give us time from smoke tests to verify the progress of the
        # agents job
        if job.operation() == "reg":
            time.sleep(10)

        resp.status().meta(resp.status().name(), content_id)
        return Worker.WORKER_SUCCESS


if __name__ == "__main__":
    #import pydevd
    #pydevd.settrace('192.168.86.1', port=9876, stdoutToServer=True,
    #                stderrToServer=True)

    worker = WorkerTestAgentsPython(sys.argv[2])
    worker.run()
