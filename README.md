# librdesc
*deterministic recursive descent parser with ordered-choice semantics*

`librdesc` is a portable and table-driven parsing library written in
standard C99. It provides the flexibility of a recursive descent parser with
high degree of control in manual stack management.

*Check out [online documentation](https://metwse.github.io/rdesc/) for details!*

## Documentation
Full API documentation is auto-generated using Doxygen:
```sh
make docs
```
## Integration Guide
You can easily embed `librdesc` as a subproject and build dependency inside
your own Makefiles by including the [rdesc.mk](./rdesc.mk).

This avoids the need for system-wide installation and builds the library
alongside your project.

```makefile
# Set the path to your rdesc source folder. (REQUIRED)
RDESC_DIR := path/to/rdesc

# Optionally configure the build variables
RDESC_MODE := debug
RDESC_FEATURES := stack dump_cst

# You may prefer installing rdesc using git
$(RDESC_DIR)/rdesc.mk:
	git clone https://github.com/metwse/rdesc.git $(RDESC_DIR)

# Include the librdesc build system
include $(RDESC_DIR)/rdesc.mk

# ...

# Use the exported variables in your targets. $(RDESC) points to the static
# library path.
my_app: main.c $(RDESC)
	$(CC) -I$(RDESC_INCLUDE_DIR) $< $(RDESC) -o $@
```

### Configuration Variables
`librdesc` provides an include-based build which uses the following `RDESC_*`
configuration variables to control the build process. When building directly
from this repository's root makefile, you can drop the `RDESC_` prefix (e.g.,
`make MODE=debug`).

| Variable | Description | Default | Valid Values |
|----------|-------------|---------|--------------|
| `RDESC_MODE` | Determines the optimization level and instrumentation. | `release` | `release`, `debug`, `test` |
| `RDESC_FEATURES` | Toggles modules linked into the library. | `stack` | `stack`, `dump_bnf`, `dump_cst`, `full` |
| `RDESC_FLAGS` | Internal flags to configure library behavior. | `ASSERTIONS` | `ASSERTIONS`, `full` |
| `RDESC_DIR` | Path to the root of the `librdesc` source repository. | `.` (*do not* use default) | rdesc path |

`rdesc.mk` defines two target variables: `RDESC`, the static library target and
`RDESC_SO`, the shared object version. You can set these before including the
`rdesc.mk` file to output into desired location, but you can also use the
default values that output to rdesc's internal build directory.

A variable named `RDESC_INCLUDE_DIR` is also defined to point to the folder
containing the public headers.

## `contribute -Wai-slop`
<img width="96" height="96" alt="no-ai-slop" align="right" src="https://github.com/user-attachments/assets/bca16d5a-a6fe-4cbf-b41f-1176e000cff2" />

Contributions are welcome! Please check our
[Code of Conduct](http://github.com/metwse/code-of-conduct) before submitting
pull requests.
