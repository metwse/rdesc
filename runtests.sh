#!/bin/bash

set -e
set -x # Komutları loglarda göster

# Build tests
make -C tests all -j
make -C examples -j

mkdir -p ./dist/tests

# Run tests
for test in $(ls ./dist/tests/*.test 2>/dev/null); do
    valgrind --error-exitcode=1 $test > $test.log
done

valgrind --error-exitcode=1 dist/examples/bc_interactive > dist/examples/bc.log <<EOF
1.0 + .2;
2 / +2;
1 * (-1 + 2);
1 +++
1 - invalid_token
2;
EOF

gcovr --root . --exclude tests/ --exclude examples/
