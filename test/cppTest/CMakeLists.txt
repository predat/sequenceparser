# ─────────────────────────────────────────────────────────────────────────────
# Catch2 — fetched at configure time so no system package is required.
# FIND_PACKAGE_ARGS lets a system-installed Catch2 satisfy it when present.
# ─────────────────────────────────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1
    GIT_SHALLOW    TRUE
    FIND_PACKAGE_ARGS 3
)

# Build Catch2 as a static library regardless of the project's BUILD_SHARED_LIBS:
# it is a test-only dependency, and as a DLL it would hit the same MSVC
# IPO + WINDOWS_EXPORT_ALL_SYMBOLS linker crash we already avoid for our own
# shared library. Static also spares us putting Catch2.dll on PATH at test time.
# Restore the flag right after so it still drives our own library selection below.
set(_sp_saved_build_shared "${BUILD_SHARED_LIBS}")
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(Catch2)
set(BUILD_SHARED_LIBS "${_sp_saved_build_shared}")

# Library under test: the shared library by default, the static one otherwise.
if(BUILD_SHARED_LIBS)
    set(_sp_lib sequenceparser-shared)
else()
    set(_sp_lib sequenceparser-static)
endif()

add_executable(sequenceParserCppTests
    testFrameRange.cpp
    testSequence.cpp
    testDecomposeFilename.cpp
    testBrowse.cpp
)
target_link_libraries(sequenceParserCppTests PRIVATE ${_sp_lib} Catch2::Catch2WithMain)

# On Windows the test executable must sit next to sequenceParser.dll so the
# loader — and Catch2's post-build test discovery — can find it at run time.
if(WIN32)
    set_target_properties(sequenceParserCppTests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:${_sp_lib}>)
endif()

# Register every Catch2 TEST_CASE as an individual CTest test.
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(sequenceParserCppTests)
