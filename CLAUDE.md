# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A C++17 library that detects numbered file **sequences** (e.g. `frame.0001.exr`, `frame.0002.exr`)
on the filesystem — the VFX / image-sequence use case. Python and Java bindings are generated with
SWIG. The branch `feature/remove_boost` migrated the codebase off Boost onto the C++17 standard
library (`std::filesystem`, `std::regex`, `std::gcd`).

## Build

CMake + Ninja. The build tree lives in `build/`.

```bash
# Configure (once)
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release

# Build everything (C++ lib + SWIG bindings)
cmake --build build          # or: ninja -C build

# Rebuild only the Python binding after editing a .i file
cmake --build build --target sequenceParser
```

Notable CMake options: `BUILD_SHARED_LIBS` (default ON), `BUILD_STATIC_LIBS` (OFF),
`SEQUENCEPARSER_COVERAGE` (OFF). IPO/LTO is enabled automatically when the toolchain supports it.
The library version is defined **only** in `src/sequenceParser/common.hpp` (the
`SEQUENCEPARSER_VERSION_*` macros); the top-level `CMakeLists.txt` parses it from there, so bump the
version in that header.

## Tests

There are two test suites: native **C++ unit tests** (Catch2) and **Python** integration tests
(pytest).

### C++ tests (Catch2 + CTest)

Sources live in `test/cppTest/`. Catch2 v3 is pulled in with `FetchContent` at configure time (no
system package needed), and every `TEST_CASE` is registered with CTest via `catch_discover_tests`.
They are built by default (`-DSEQUENCEPARSER_BUILD_TESTS=ON`) and link the library target directly,
so they exercise the public API *and* internal `detail/` helpers (`decomposeFilename`, …) without
going through SWIG.

```bash
cmake --build build                       # builds sequenceParserCppTests too
ctest --test-dir build --output-on-failure        # run all C++ tests
ctest --test-dir build -R FrameRange              # run a subset by name
```

### Python tests

Python tests in `test/pyTest/` run under **pytest** (migrated away from nose). A `venv/` with pytest
is expected at the repo root.

```bash
venv/bin/python -m pytest                                   # all tests
venv/bin/python -m pytest test/pyTest/testStat.py           # one module
venv/bin/python -m pytest test/pyTest/testStat.py::testFileStat   # one test
```

Tests import the library as `from pySequenceParser import sequenceParser as seq`. They run directly
against the build tree (no install needed) because `conftest.py` synthesises a `pySequenceParser`
package whose `__path__` points at both `build/src/` (the `_sequenceParser.so` C extension) and
`build/src/sequenceParser/` (the generated `sequenceParser.py` wrapper) — the SWIG wrapper does a
relative `from . import _sequenceParser`, so both must appear as siblings. `pytest.ini` sets
`python_files = test*.py` because the test files use the `testXxx.py` naming convention rather than
pytest's default `test_*.py`. **Rebuild the binding before running tests** if you changed C++ or `.i`
files.

## Code style

`.clang-format` and `.clang-tidy` define the C++ conventions:
- Identifier naming (enforced by clang-tidy): functions/variables `camelBack`, classes/structs
  `CamelCase`.
- Run `clang-format` on C++ files before committing.

## Architecture

### Public C++ API

Everything lives in namespace `sequenceParser`. The two entry points are in `filesystem.hpp`:

- `browse(directory, detectOptions, filters)` → `std::vector<Item>`. Scans a directory and returns
  files, folders, links and detected sequences as `Item` objects. This is the main interaction with
  the filesystem; the rest of the API manipulates the returned `Item`s without further disk access.
- `browseSequence(outSequence, pattern, accept)` → `bool`. Builds a single `Sequence` from a known
  pattern path like `/tmp/foo####.jpg` without a full directory browse.

### Core domain objects

- **`Item`** (`Item.hpp`) — the universal filesystem element: file, folder, link, or sequence.
  `getType()` returns an `EType`; for `eTypeSequence`, `getSequence()` yields the descriptor and
  `explode()` expands it into one `Item` per frame.
- **`Sequence`** (`Sequence.hpp`) — a numbered sequence modelled as `prefix + zero-padded number +
  suffix`, plus a list of `FrameRange`s. Has **no** parent-folder notion (the folder is carried by
  the wrapping `Item`). Padding is tri-state: *fixed* (`getFixedPadding()` > 0, all frames same
  width), *variable* (mixed widths → `getFixedPadding()` == 0, `getMaxPadding()` > 0), and *unknown*
  (no frame has a leading zero).
- **`FrameRange`** (`FrameRange.hpp`) — a `(first, last, step)` triple. A sequence with holes is
  represented as multiple ranges. `getFramesIterable()` provides iteration over frame numbers.
- **`ItemStat`** (`ItemStat.hpp`) — `stat()`-style filesystem metadata (size, sizeOnDisk, hard
  links, owner/group, permissions, times) computed from an `Item`.

### Detection pipeline

The sequence-detection logic lives in `detail/` (internal, not part of the public API):

1. `filesystem.cpp::browse` iterates the directory.
2. For each filename, `detail/analyze.cpp::decomposeFilename` splits it into alternating string and
   number parts (e.g. `aa1b22c3` → strings `[aa, b, c]`, numbers `[1, 22, 3]`) using a static
   `std::regex`.
3. Filenames sharing the same **string parts** are grouped — `detail/FileStrings` is the grouping
   key (hashed via `SeqIdHash`), `detail/FileNumbers` holds the per-file numbers.
4. `detail/analyze.cpp::buildSequences` decides which numeric position varies, splits ambiguous
   multi-padding / multi-sequence cases, and emits `Sequence` objects. `EDetection` flags steer the
   ambiguous cases.

### Enums (`common.hpp`, `Sequence.hpp`)

- `EType` (`common.hpp`) — bit flags for item kind (`eTypeFile`, `eTypeFolder`, `eTypeSequence`,
  `eTypeLink`, `eTypeAll`).
- `EDetection` (`common.hpp`) — combinable detection options (`eDetectionNegative`,
  `eDetectionSequenceNeedAtLeastTwoFiles`, `eDetectionIgnoreDotFile`, …, `eDetectionDefault`).
- `EPattern` (`Sequence.hpp`) — accepted pattern styles: standard `####`/`@`, C-style `%04d`, frame,
  negative-frame.
- `Time` is `std::ssize_t` — frame numbers can be negative.

## SWIG bindings — important

Each C++ header has a sibling `.i` interface file (`Sequence.hpp` ↔ `Sequence.i`). `sequenceParser.i`
is the top-level module that `%include`s the others.

- **Never edit the generated `build/src/sequenceParser/sequenceParser.py`** — it is regenerated on
  every build. Edit the `.i` source and rebuild.
- Python docstrings are added in the `.i` files via `%feature("docstring") ClassName::method "..."`
  placed **before** the `%include "Header.hpp"`. The module docstring is in `sequenceParser.i`'s
  `%pythonbegin` block.
- Pythonic additions (`__len__`, `__bool__`, `__contains__`, etc.) are added via
  `%extend ... %pythoncode` inside `#ifdef SWIGPYTHON` guards.
- `Sequence::isIn` uses SWIG `OUTPUT` typemaps, so in Python it returns a tuple/list
  `[found, frame_number, frame_string]` rather than taking output parameters.
- Language-specific renames (`__str__`, `__eq__`, `toString`, `equals`) are guarded with
  `#ifdef SWIGJAVA` / `#elif SWIGPYTHON`.

## Reference docs

`USAGE.md` has a thorough Python-oriented API walkthrough (padding semantics, browse options,
`explode()`). `INSTALL.md` covers the build. `examples/` has runnable Python scripts
(`lsSequence.py`, `lsFile.py`, `lsFolder.py`).
