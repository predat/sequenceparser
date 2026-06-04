"""pytest configuration: wire up the in-tree build as the pySequenceParser package.

The CMake build puts:
  build/src/_sequenceParser.so       – compiled C extension
  build/src/sequenceParser/sequenceParser.py  – SWIG Python wrapper

The SWIG wrapper uses `from . import _sequenceParser` (relative import), so both
files must be seen as siblings inside the same package.  At install time CMake
places them together under pySequenceParser/; in the build tree we replicate
that by constructing a synthetic pySequenceParser package whose __path__ covers
both directories.
"""
import os
import sys
import types

_root = os.path.dirname(os.path.abspath(__file__))
_build_src = os.path.join(_root, "build", "src")

# On Windows the compiled extension (_sequenceParser.pyd) depends on
# sequenceParser.dll sitting next to it. Since Python 3.8 the directory of an
# extension module is no longer searched automatically for its dependent DLLs,
# so register it explicitly before the import happens.
if sys.platform == "win32" and os.path.isdir(_build_src):
    os.add_dll_directory(_build_src)

_pkg = types.ModuleType("pySequenceParser")
_pkg.__path__ = [
    os.path.join(_build_src, "sequenceParser"),  # sequenceParser.py lives here
    _build_src,                                   # _sequenceParser.so lives here
]
_pkg.__package__ = "pySequenceParser"
sys.modules["pySequenceParser"] = _pkg
