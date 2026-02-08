#!/bin/bash

make all

for test in $(ls dist/*.test); do
    valgrind $test > $test.log
done

valgrind dist/bc_interactive > dist/bc.log <<EOF
1.0 + .2;
2 / +2;
1 * (-1 + 2);
1 +++
1 - invalid_token
2;
EOF

gcovr
