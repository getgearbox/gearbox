#!/usr/bin/env python

import glob
import json
import os
import sys

from gearbox import Worker, ERR_NOT_FOUND, ERR_BAD_REQUEST


def file_get_contents(filename):
    with open(filename, 'r') as f:
        contents = f.read()
    return contents


def file_put_contents(filename, contents):
    with open(filename, 'w') as f:
        f.write(contents)


class WorkerTestBasicPython(Worker):
    DBDIR = "/var/gearbox/db/test-basic-python/"

    def __init__(self, cfg):
        super(WorkerTestBasicPython, self).__init__(cfg)

        self.register_handler("do_get_testbasicpython_thing_v1")
        self.register_handler("do_put_testbasicpython_thing_v1")
        self.register_handler("do_post_testbasicpython_thing_v1")
        self.register_handler("do_delete_testbasicpython_thing_v1")

    def do_get_testbasicpython_thing_v1(self, job, resp):
        env = job.environ()
        if "TestBasic" in env:
            resp.add_header("TestBasic", env["TestBasic"])

        args = job.arguments()

        if not args:  # index GET
            files = glob.glob(self.DBDIR + "*")
            out = {"things": []}
            limit = len(files)

            if "_count" in job.query_params():
                cgi = job.query_params()
                limit = int(cgi["_count"])

            #TODO: this isn't using limit like perl/php does
            for i in xrange(limit):
                matrix = job.matrix_arguments()
                if "_expand" in matrix and matrix["_expand"] == "1":
                    contents = file_get_contents(files[i])
                    out["things"].append(json.loads(contents))
                else:
                    out["things"].append(os.path.basename(files[i]))

            resp.content(json.dumps(out))
        else:
            name = args[0]
            file_path = os.path.join(self.DBDIR, name)
            if os.path.exists(file_path):
                resp.content(file_get_contents(file_path))
            else:
                raise ERR_NOT_FOUND('thing "%(name)s" not found' % locals())

        return Worker.WORKER_SUCCESS

    def do_put_testbasicpython_thing_v1(self, job, resp):
        # async message, so update status to let user know we are processing
        resp.status().progress(10)
        resp.status().add_message("processing")
        args = job.arguments()
        if not args:
            raise ERR_BAD_REQUEST("missing required resource name")

        job_content = json.loads(job.content())
        if not "id" in job_content:
            raise ERR_BAD_REQUEST('missing required "id" field')

        filename = os.path.join(self.DBDIR, args[0])
        file_put_contents(filename, job.content())
        resp.status().add_message("done")

        return Worker.WORKER_SUCCESS

    def do_post_testbasicpython_thing_v1(self, job, resp):
        resp.status().progress(10)
        resp.status().add_message("processing")

        if job.operation() == "create":
            # post-create where the resource id is created for user
            # (instead of a PUT where the user specifies the name)

            # get the generated id
            job_content = json.loads(job.content())
            job_content["id"] = job.resource_name()

            filename = os.path.join(self.DBDIR, job.resource_name())
            file_put_contents(filename, json.dumps(job_content))
        else:
            args = job.arguments()
            # post update
            filename = os.path.join(self.DBDIR, args[0])
            if os.path.exists(filename):
                job_content = json.loads(job.content())
                out = json.loads(file_get_contents(filename))
                out["stuff"] = job_content["stuff"]
                file_put_contents(filename, json.dumps(out))
            else:
                raise ERR_NOT_FOUND('thing "%s" not found' % args[0])
        resp.status().add_message("done")
        return Worker.WORKER_SUCCESS

    def do_delete_testbasicpython_thing_v1(self, job, resp):
        resp.status().progress(10)
        resp.status().add_message("processing")

        # don't actually delete if fake-out header is set
        headers = job.headers()
        if "fake-out" in headers and int(headers["fake-out"]) == 1:
            resp.status().add_message("ignoring delete due to fake-out header")
            return Worker.WORKER_SUCCESS

        args = job.arguments()
        if not args:
            raise ERR_BAD_REQUEST("missing required resource name")

        filename = os.path.join(self.DBDIR, args[0])
        if os.path.exists(filename) and os.path.isfile(filename):
            os.unlink(filename)
        else:
            raise ERR_NOT_FOUND('thing "%s" not found' % args[0])

        resp.status().add_message("done")
        return Worker.WORKER_SUCCESS


if __name__ == "__main__":
    worker = WorkerTestBasicPython(sys.argv[2])
    worker.run()
