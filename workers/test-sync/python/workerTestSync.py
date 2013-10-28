#!/usr/bin/env python

import glob
import json
import os
import sys

import gearbox
from gearbox import Worker


def file_get_contents(filename):
    with open(filename, 'r') as f:
        contents = f.read()
    return contents


def file_put_contents(filename, contents):
    with open(filename, 'wb') as f:
        f.write(contents)


class WorkerTestSyncPython(Worker):
    DBDIR = "/var/gearbox/db/test-sync-python/"

    def __init__(self, config):
        super(WorkerTestSyncPython, self).__init__(config)

        self.register_handler("do_get_testsyncpython_thing_v1")
        self.register_handler("do_put_testsyncpython_thing_v1")
        self.register_handler("do_post_testsyncpython_thing_v1")
        self.register_handler("do_delete_testsyncpython_thing_v1")

    def do_get_testsyncpython_thing_v1(self, job, resp):
        env = job.environ()
        if "TestSync" in env:
            resp.add_header("TestSync", env["TestSync"])

        args = job.arguments()
        if not args:
            files = glob.glob(self.DBDIR + "*")
            out = {"things": []}

            # set things to an empty array in case our glob did not match
            # anything
            limit = 10

            if "_count" in job.query_params():
                cgi = job.query_params()
                limit = int(cgi["_count"])

            count = 0
            for f in files:
                count += 1
                if count > limit:
                    break

                matrix = job.matrix_arguments()
                if "_expand" in matrix and matrix["_expand"] == "1":
                    file_contents = json.loads(file_get_contents(f))
                    out["things"].append(file_contents)
                else:
                    out["things"].append(os.path.basename(f))

            # set the output content
            resp.content(json.dumps(out))
        else:
            name = args[0]
            file_name = os.path.join(self.DBDIR, name)
            if os.path.exists(file_name):
                resp.content(file_get_contents(file_name))
            else:
                raise gearbox.ERR_NOT_FOUND('thing "%s" not found' % name)

        return Worker.WORKER_SUCCESS

    def do_put_testsyncpython_thing_v1(self, job, resp):
        args = job.arguments()
        if not args:
            raise gearbox.ERR_BAD_REQUEST("missing required resource name")

        job_content = json.loads(job.content())
        if not "id" in job_content:
            raise gearbox.ERR_BAD_REQUEST('missing required "id" field')

        file_put_contents(os.path.join(self.DBDIR, args[0]),
                          job.content())

        resp.content(job.content())
        return Worker.WORKER_SUCCESS

    def do_post_testsyncpython_thing_v1(self, job, resp):

        job_content = json.loads(job.content())

        if job.operation() == "create":
            # post-create where the resource id is created for user
            # (instead of a PUT where the user specifies the name)

            # get the generated id
            job_content["id"] = job.resource_name()
            content = json.dumps(job_content)
            resource_file = os.path.join(self.DBDIR, job.resource_name())
            file_put_contents(resource_file, content)
            resp.content(content)
        else:
            args = job.arguments()
            # post update
            file_name = os.path.join(self.DBDIR, args[0])

            if os.path.exists(file_name):
                out = json.loads(file_get_contents(file_name))
                out["stuff"] = job_content["stuff"]
                content = json.dumps(out)
                file_put_contents(file_name, content)
                resp.content(content)
            else:
                raise gearbox.ERR_NOT_FOUND('thing "%s" not found' % args[0])

        return Worker.WORKER_SUCCESS

    def do_delete_testsyncpython_thing_v1(self, job, resp):
        # don't actually delete if fake-out header is set
        headers = job.headers()
        if "fake-out" in headers and int(headers["fake-out"]) == 1:
            return Worker.WORKER_SUCCESS

        args = job.arguments()
        if not args:
            raise gearbox.ERR_BAD_REQUEST("missing required resource name")

        file_name = os.path.join(self.DBDIR, args[0])
        if os.path.exists(file_name) and os.path.isfile(file_name):
            os.unlink(file_name)
        else:
            raise gearbox.ERR_NOT_FOUND('thing "%s" not found' % args[0])

        return Worker.WORKER_SUCCESS

if __name__ == "__main__":
    #import pydevd
    #pydevd.settrace('192.168.86.1', port=9876, stdoutToServer=True,
    #                stderrToServer=True)

    worker = WorkerTestSyncPython(sys.argv[2])
    worker.run()
