#!/bin/bash

make -C tests covr -j

for test in $(ls ./dist/tests/*.covr); do
    valgrind $test > $test.log
done

gcovr *
