#!/usr/bin/env python

import setup

from testtrivial import *
from gearbox import HttpClient

PLAN(6)

# not much here, we really only import the consts
# for use with the JobManager.job(HttpClient::Method, Uri) api

IS( HttpClient.METHOD_GET, 0 )
IS( HttpClient.METHOD_DELETE, 1 )
IS( HttpClient.METHOD_POST, 2 )
IS( HttpClient.METHOD_PUT, 3 )
IS( HttpClient.METHOD_HEAD, 4 )
IS( HttpClient.METHOD_UNKNOWN, 5 )
