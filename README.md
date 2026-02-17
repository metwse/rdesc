# librdesc
generic descent parser

`librdesc` is a portable, table-driven, algorithm-only parsing library written
in standard C99. It provides the flexibility of a recursive descent parser with
high degree of control in manual stack management.

*Check out [online documentation](https://metwse.github.io/rdesc/) for details!*


## Documentation
Full API documentation is auto-generated using Doxygen:
```sh
make docs
```


## `contribute -Wai-slop`
<img width="96" height="96" alt="no-ai-slop" align="right" src="https://github.com/user-attachments/assets/bca16d5a-a6fe-4cbf-b41f-1176e000cff2" />

Contributions are welcome! Please check our
[Code of Conduct](http://github.com/metwse/code-of-conduct) before submitting
pull requests.


## Building
`librdesc` uses a modular build system with separate Makefiles for the library,
tests, and examples.

### Library
Build the main library:
```sh
make              # Build release version (default)
make MODE=debug   # Build with debug symbols and ASAN
make MODE=test    # Build with coverage instrumentation
```

### Feature Flags
Providing `FEATURES` variable, you can toggle modules linked to the library. By
default, only `stack` feature is enabled.

```sh
make FEATURES='stack dump_bnf dump_cst'
```

| Feature | Description |
|--|--|
| `stack` | Use built-in stack implementation in backtracing, which uses `malloc/free` family functions. |
| `dump_bnf` | Dump `rdesc_gramar` in Backus-Naur form. |
| `dump_cst` | Dump `rdesc_node` (Concrete Syntax Tree) as dotlang graph. |

You may use feature flag `full` to include all features.

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

### Manual integration
Copy `include/*.h` to your project and static link against `librdesc.a` or
dynamic link against `librdesc.so`.
