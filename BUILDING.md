# Manual Building
`librdesc` uses a modular build system with separate Makefiles for the library,
tests, and examples.

### Library
Build the main library:
```sh
make              # Build release version (default)
make MODE=debug   # Build with debug symbols and ASAN
make MODE=test    # Build with coverage instrumentation
```

### Features
Providing `FEATURES` variable, you can toggle modules linked to the library. By
default, only `stack` feature is enabled. You may use feature flag `full` to
include all features.

```sh
make FEATURES='stack dump_bnf dump_cst'
```

| Feature | Description |
|--|--|
| `stack` | Use built-in stack implementation in backtracing, which uses `malloc/free` family functions. |
| `dump_bnf` | Dump `rdesc_grammar` in Backus-Naur form. |
| `dump_cst` | Dump `rdesc_node` (Concrete Syntax Tree) as dotlang graph. |

### Flags
Providing `FLAGS` variable, you can toggle injection macros. Similar to
Features, you may use feature flag `full` to include all flags.

| Flag | Description |
|--|--|
| `ASSERTIONS` | Enable runtime boundary and logic validation checks. |

### Tests
Tests are organized into three categories and built independently:
```sh
cd tests
make covr   # Build coverage-instrumented tests (integration, unit, fuzz)
make debug  # Build debug tests with ASAN (integration, unit)
make fuzz   # Build optimized fuzz tests
```

Or from project root:
```sh
./runtests.sh   # Build and run all tests with coverage
```

### Examples
Examples build using their Makefile, and example output will be present in
`dist/examples/` folder.
```sh
cd examples
make            # Build example programs
```

Examples statically links `librdesc` in tests mode.


## Installation
```sh
make install PREFIX=/installation/path
```

This command copies headers into `$PREFIX/include` and `librdesc.so/a` into
`$PREFIX/lib`.

Leave it empty to install `librdesc` system-wide:
```sh
sudo make install
```

Also you can specify build mode and features for installation:
```sh
sudo make install FEATURES=full MODE=debug
```
Note: Test mode is not recommended for installation.


## Manual Integration
Copy `include/*.h` to your project and static link against `librdesc.a` or
dynamic link against `librdesc.so`.
