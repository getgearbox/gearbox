#!/usr/bin/env python

import os
import sys

from gearbox import Worker


def file_get_contents(filename):
    with open(filename, 'r') as f:
        contents = f.read()
    return contents


def file_put_contents(filename, contents):
    with open(filename, 'wb') as f:
        f.write(contents)


class WorkerTestDelayPython(Worker):
    DBDIR = "/var/gearbox/db/test-delay-python/"

    def __init__(self, config):
        super(WorkerTestDelayPython, self).__init__(config)
        self.register_handler("do_get_testdelaypython_counter_v1")
        self.register_handler("do_post_testdelaypython_counter_v1")
        self.register_handler("do_delete_testdelaypython_counter_v1")
        self.register_handler("do_increment_testdelaypython_counter_v1")

    def do_get_testdelaypython_counter_v1(self, job, resp):
        resource_file = os.path.join(self.DBDIR, job.resource_name())
        resp.content(file_get_contents(resource_file))
        return Worker.WORKER_SUCCESS

    def do_post_testdelaypython_counter_v1(self, job, resp):
        matrix = job.matrix_arguments()
        #grab start if available, else default to 0
        start = matrix.get("start", "0")

        resource_file = os.path.join(self.DBDIR, job.resource_name())
        file_put_contents(resource_file, start)

        seconds = int(matrix.get("delay", "1"))
        self.afterwards(job, "do_increment_testdelaypython_counter_v1",
                        seconds)
        return Worker.WORKER_CONTINUE

    def do_delete_testdelaypython_counter_v1(self, job, resp):
        args = self.arguments()
        os.unlink(os.path.join(self.DBDIR, args[0]))
        return Worker.WORKER_SUCCESS

    def do_increment_testdelaypython_counter_v1(self, job, resp):
        resource_file = os.path.join(self.DBDIR, job.resource_name())
        newval = 1 + int(file_get_contents(resource_file))

        file_put_contents(resource_file, str(newval))
        matrix = job.matrix_arguments()
        start = int(matrix.get("start", "0"))
        end = int(matrix.get("end", "10"))

        resp.status().add_message("set to %s" % newval)
        if newval == end:
            return Worker.WORKER_SUCCESS
        else:
            resp.status().progress(int(resp.status().progress()) +
                                   (end - start))

        matrix = job.matrix_arguments()
        seconds = int(matrix.get("delay", "1"))

        if "retry" in matrix and matrix["retry"]:
            msg = "retry attempt number %s" % \
                  (int(resp.status().failures()) + 1)
            resp.status().add_message(msg)
            return Worker.WORKER_RETRY
        else:
            self.afterwards(job, seconds)

        return Worker.WORKER_CONTINUE


if __name__ == "__main__":
    worker = WorkerTestDelayPython(sys.argv[2])
    worker.run()
