#!/bin/bash

# keep track of the working directory
CWD=$(pwd)

# unset PREFIX environment variable for c++ workers
unset PREFIX

# run the smoke tests for c++ workers
for filepath in $CWD/workers/test-*
do
    cd $filepath/smoke
    ./test.t
done

# set PREFIX environment variable to perl for perl workers
export PREFIX=perl

# run the smoke tests for perl workers
for filepath in $CWD/workers/test-*
do
    cd $filepath/smoke
    ./test.t
done
#!/bin/bash

# set PREFIX environment variable to python for python workers
export PREFIX=python

# run the smoke tests for python workers
cd $CWD/workers/test-basic/smoke
./test.t
