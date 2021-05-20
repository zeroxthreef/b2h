# b2h
b2h is a public domain minimal binary to header util that you can still embed into another program or use by itself. Just a single function and, when compiling standalone, it is its own utility. Written in an hour because I was tired of making ``xxd`` a dependency in my build scripts.

b2h automatically appends a null terminator regardless of input. This will affect sizeof on the array var so the actual length is the ``<file-out-identifier>_len`` var.

b2h supports files over 4gb, but it has to run on a 64-bit machine.
## Standalone Usage
``c2d <file-in> <file-out>``

## Embedding
```CPP
#define B2H_IMPLEMENTATION
#include "b2h.c"
...
header_data = b2h("file_name.txt", file_data, file_data_len);
```

## License
Public Domain
Unlicense (international public domain)