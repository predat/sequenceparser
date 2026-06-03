# How to install sequenceParser

#### External dependencies
* Swig
Version 4.1.0 or upper
(for Python/Java Binding)

#### To build
SequenceParser uses CMake build system.
```
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build
```

#### Tested compilers
###### Linux
* GCC 11 on Centos 7 64bits
* LLVM 22 on Fedora 44

###### MacOS

###### Windows

#### Launch tests
###### Python tests
Using nosetests:
```
nosetests test/pyTest
```
