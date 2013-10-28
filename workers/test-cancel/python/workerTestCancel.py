#!/usr/bin/env python

import sys
import time

from gearbox import Job
from gearbox import Status
from gearbox import Worker


class WorkerTestCancelPython(Worker):
    DBDIR = "/usr/var/gearbox/db/test-cancel-py/"

    def __init__(self, cfg):
        super(WorkerTestCancelPython, self).__init__(cfg)

        self.register_handler("do_post_testcancelpython_thing_v1")
        self.register_handler("do_cancel_testcancelpython_thing_v1")

        self.register_handler("do_post_testcancelpython_continuation_v1")
        self.register_handler("do_run_testcancelpython_continuation_v1")
        self.register_handler("do_finish_testcancelpython_continuation_v1")

    def do_post_testcancelpython_thing_v1(self, job, resp):
        s = resp.status()
        on_cancel = self.job_manager().job(
            "do_cancel_testcancelpython_thing_v1")
        s.on(Status.EVENT_CANCEL, on_cancel)

        stop = time.time() + 30
        while stop >= time.time():
            s.sync()
            p = s.progress()
            if p < 100:
                p += 10
                s.progress(p)
                s.checkpoint()
                time.sleep(5)
        else:
            return Worker.WORKER_SUCESS

    def do_cancel_testcancelpython_thing_v1(self, job, resp):
        resp.status().add_message("on cancel callback called")
        return Worker.WORKER_SUCCESS

    def do_post_testcancelpython_continuation_v1(self, job, resp):
        # we have do_post with a continuation of do_finish.  The do_finish
        # is only called via on-completion handler for do_run
        run = self.job_manager().job("do_run_testcancelpython_continuation_v1")
        finish = Job(job)
        finish.name("do_finish_testcancelpython_continuation_v1")
        run.on(Job.EVENT_COMPLETED, finish)
        self.afterwards(run)
        return Worker.WORKER_CONTINUE

    def do_run_testcancelpython_continuation_v1(self, job, resp):
        resp.status().add_message("run called")
        # self will retry indefinitely, we want to test the cancellation of an
        # status in progress that is suspended waiting upon child completion
        # events
        return Worker.WORKER_RETRY

    def do_finish_testcancelpython_continuation_v1(self, job, resp):
        # self will never get called since do_run will never complete
        return Worker.WORKER_SUCCESS

if __name__ == "__main__":
    worker = WorkerTestCancelPython(sys.argv[2])
    worker.run()
