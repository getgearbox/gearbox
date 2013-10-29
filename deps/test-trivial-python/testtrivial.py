#!/usr/bin/env python

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

