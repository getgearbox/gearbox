#!/usr/bin/env python

#################################################################################
# Copyright (c) 2014, Jay Buffington
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * The names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#################################################################################

from TAP.Simple import *
import inspect
import re

class Vars(dict):
    def set(self,k,v):
        super(Vars, self).__setitem__(k,v)
        return v

def line_to_text(level=1):
    # FIXME
    # this seems to return only the last line of the caller so statements like:
    # OK(
    #      foobar() == 5
    # )
    # don't work
    lines = inspect.stack()[level][4]
    source = "".join(lines)

    # FIXME, this doesn't match the matching paren, so lines like this:
    #     IS( str(THROWS( lambda: raise Exception('whops!') )), 'whops!' )
    # aren't parsed properly
    m = re.match(r"\s*(OK)\s*\((.*)\)", source)
    if m:
        text = m.group(2)
    else:
        text = source

    return text.rstrip()

def PLAN(test_count):
    return plan(test_count)

def OK(test, msg=None):
    msg = msg or line_to_text(2)
    ok_ret = ok(test, msg)
    return (ok_ret, test)

def NOK(test, msg=None):
    msg = msg or line_to_text(2)
    return ok(not test, msg)

def IS(value, expected, msg=None):
    msg = msg or line_to_text(2)
    return eq_ok( value, expected, msg )

def LIKE(test, regex, msg=None):
    msg = msg or line_to_text(2)
    m = re.match(regex, test)
    if m:
        return ok(1, msg)
    else:
        return ok(0, msg)

def THROWS(test, msg=None):
    msg = msg or line_to_text(2)
    try:
        test()
    except Exception as err:
        ok( 1, msg )
        return err

    return ok( 0, msg )

def NOTHROW(test, msg=None):
    msg = msg or line_to_text(2)
    try:
        test_ret = test()
    except Exception as err:
        ok( 0, msg )
        diag( str(err) )
        return err

    return (ok( 1, msg ), test_ret)

