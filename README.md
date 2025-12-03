# librdesc
generic right-recursice descent parser

`librdesc` is an portable, table-driven algorithm-only parsing library written
in standard C99. It provides the flexibility of a recursive descent parser with
high degree of control in manual stack management.


## `contribute -Wai-slop`
<img width="96" height="96" alt="no-ai-slop" align="right" src="https://github.com/user-attachments/assets/bca16d5a-a6fe-4cbf-b41f-1176e000cff2" />

Contributions are welcome! Please check our
[Code of Conduct](http://github.com/metwse/code-of-conduct) before submitting
pull requests.


## Building
You can build `librdesc` and its utilities using GNUMake.
```sh
make        # Build the main library
make tests  # Build all test binaries
```

### Feature Flags
Providing `FEATURES` variable, you can toggle modules linked to the library. By
default, only `stack` feature is enabled.

```sh
make FEATURES='stack dump_bnf dump_dot'
```

| Feature | Description |
|--|--|
| `stack` | Use built-in stack implementation in backtracing, which uses `malloc/free` family functions. |
| `dump_bnf` | Dump `rdesc_cfg` (Context-Free Grammar) in Backus–Naur form. |
| `dump_dot` | Dump `rdesc_node` (Concrete Syntax Tree) as dotlang graph. |

## Documentation
Documentations can be auto-generated using Doxygen.
