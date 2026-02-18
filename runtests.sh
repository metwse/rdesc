#!/bin/bash

make -C tests -j
make -C examples all -j

for test in $(ls ./dist/tests/*.test); do
    valgrind $test > $test.log
done

valgrind dist/examples/bc_interactive > dist/examples/bc.log <<EOF
1.0 + .2;
2 / +2;
1 * (-1 + 2);
1 +++
1 - invalid_token
2;
EOF


gcovr *
