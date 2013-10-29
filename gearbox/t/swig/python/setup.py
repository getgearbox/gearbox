from os.path import dirname, abspath, realpath
from os import environ, execve, chdir
import sys

path = dirname(abspath(__file__))
PATH=":".join(map(lambda module: realpath(path + "../../../" + module + "/.libs"), "core job worker store".split()))

if ( environ.get('LD_LIBRARY_PATH') !=  PATH ):
    environ['LD_LIBRARY_PATH'] = PATH

    stubpath = realpath(path + "../../../../../common/stub")
    environ['LD_PRELOAD'] = stubpath + "/.libs/libgearman_stub.so"

    # for OS X
    environ['DYLD_LIBRARY_PATH'] = PATH;
    environ['DYLD_INSERT_LIBRARIES'] = stubpath + "/.libs/libgearman_stub.dylib";
    environ['DYLD_FORCE_FLAT_NAMESPACE'] = "1";

    environ['PYTHONPATH'] = ":".join(
        map(lambda x: realpath(path + "../../../../" + x), "../deps/test-trivial-python/ ../deps/PyTAP/ swig/python/.libs swig/python/lib".split())
    )

    script = abspath(sys.argv[0])
    execve(script, sys.argv[1:], environ)

chdir(path)
