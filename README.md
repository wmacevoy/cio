# cio

A lightweight C11 library providing an abstract character I/O interface with pluggable backends.

## Types

- **CIO** - Base interface with read/write/peek/close via function pointers
- **CIOArray** - Array-backed I/O supporting uint8, uint16, uint32 elements with dynamic growth
- **CIOFILE** - Wraps `FILE*` with buffered lookahead (peek support)
- **CIOUTF8** - UTF-8 codec layer over any byte-level CIO stream

All types return `-1` on EOF/error. `CIORead`/`CIOWrite`/`CIOPeek`/`CIOClose` work polymorphically on any CIO-based type.

## Usage

```c
#include "cio.h"

// Read from a file with peek support
CIOFILE in;
CIOFILEInit(&in, fopen("input.txt", "r"), 1); // 1 = close file on CIOClose
int first = CIOPeek(&in, 0);  // lookahead without consuming
int ch = CIORead(&in);        // consume next byte
CIOClose(&in);

// UTF-8 decoding over a byte stream
CIOArray bytes;
CIOArrayConstU8Init(&bytes, data, 0, len);
CIOUTF8 utf8;
CIOUTF8Init(&utf8, &bytes.base, 1); // 1 = close bytes on CIOClose
int codepoint = CIORead(&utf8);     // returns Unicode codepoint
CIOClose(&utf8);

// Dynamic array output
CIOArray out;
CIOArrayU8Init(&out, NULL, 0, 0, 0, 4096);
CIOWrite(&out, 'H');
CIOWrite(&out, 'i');
CIOClose(&out);
```

## Dependencies

- [libutf8](https://github.com/wmacevoy/utf8) — installed system-wide
- [facts](https://github.com/wmacevoy/facts) — vendored as git submodule (tests only)

## Build (Make)

Requires libutf8 installed (`make -C ../utf8 all && sudo make -C ../utf8 install`).

```sh
git submodule update --init   # fetch test framework
make all                      # build libcio.a + libcio.dylib/so
make check                    # run tests
sudo make install             # install to /usr/local
make clean                    # remove build artifacts
```

## Build (CMake)

```sh
cmake -B build
cmake --build build
sudo cmake --install build
```

To build tests with CMake:
```sh
cmake -B build -DCIO_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```

## Install

### From source
```sh
make && sudo make install
# or
cmake -B build && cmake --build build && sudo cmake --install build
```

### Debian / Ubuntu
```sh
sudo dpkg -i libcio-1_*.deb libcio-dev_*.deb
```

### Homebrew
```sh
brew tap wmacevoy/cio
brew install cio
```

### pkg-config
After installation, use in your build:
```sh
cc $(pkg-config --cflags --libs cio) -o myapp myapp.c
```