cmake_minimum_required(VERSION 3.16)

# ────────────────────────────────────────────────────────────────────────────
# Version – read from common.hpp so it stays the single source of truth
# ────────────────────────────────────────────────────────────────────────────
file(STRINGS "src/sequenceParser/common.hpp"
    _version_hpp REGEX "#define SEQUENCEPARSER_VERSION_")
foreach(v MAJOR MINOR MICRO)
    if(_version_hpp MATCHES "#define SEQUENCEPARSER_VERSION_${v} ([0-9]+)")
        set(SEQUENCEPARSER_VERSION_${v} "${CMAKE_MATCH_1}")
    else()
        message(SEND_ERROR "Failed to retrieve SEQUENCEPARSER_VERSION_${v} from common.hpp")
    endif()
endforeach()
unset(_version_hpp)
set(SEQUENCEPARSER_VERSION
    "${SEQUENCEPARSER_VERSION_MAJOR}.${SEQUENCEPARSER_VERSION_MINOR}.${SEQUENCEPARSER_VERSION_MICRO}")
message(STATUS "SequenceParser version: ${SEQUENCEPARSER_VERSION}")

project(sequenceParser
    VERSION ${SEQUENCEPARSER_VERSION}
    DESCRIPTION "C++ library to detect sequences, files and folders"
    LANGUAGES CXX
)

# ────────────────────────────────────────────────────────────────────────────
# Global build settings
# ────────────────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Avoid spurious rebuilds of install rules
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

# Position-independent code for all targets (required for shared libs / SWIG)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Generate compilation database (compile_commands.json).
# https://sarcasm.github.io/notes/dev/compilation-database.html#what-is-a-compilation-database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ────────────────────────────────────────────────────────────────────────────
# Options
# ────────────────────────────────────────────────────────────────────────────
# Build shared libraries by default.
get_property(DEFAULT_SHARED GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
option(BUILD_SHARED_LIBS "Build shared libraries?" ${DEFAULT_SHARED})
option(BUILD_STATIC_LIBS "Build static libraries?" OFF)

# ────────────────────────────────────────────────────────────────────────────
# Interprocedural optimisation (LTO/IPO) – enabled when supported
# ────────────────────────────────────────────────────────────────────────────
include(CheckIPOSupported)
check_ipo_supported(RESULT _ipo_supported OUTPUT _ipo_output LANGUAGES CXX)
if(_ipo_supported)
    message(STATUS "IPO/LTO: supported – enabling CMAKE_INTERPROCEDURAL_OPTIMIZATION")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
else()
    message(STATUS "IPO/LTO: not supported by this compiler/linker (${_ipo_output})")
endif()
unset(_ipo_supported)
unset(_ipo_output)

# ────────────────────────────────────────────────────────────────────────────
# Per-configuration compile options (via generator expressions – no flag pollution)
# ────────────────────────────────────────────────────────────────────────────
if(MSVC)
    add_compile_options(
        $<$<CONFIG:Debug>:/MTd>
        $<$<CONFIG:Release>:/MT>
        $<$<CONFIG:Release>:/O2>
    )
else()
    add_compile_options(
        $<$<CONFIG:Debug>:-Wall>
        $<$<CONFIG:Debug>:-Wextra>
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O3>
    )
endif()

# ────────────────────────────────────────────────────────────────────────────
# Optional: code-coverage instrumentation
# ────────────────────────────────────────────────────────────────────────────
option(SEQUENCEPARSER_COVERAGE "Enable code-coverage instrumentation" OFF)
if(SEQUENCEPARSER_COVERAGE)
    message(STATUS "Code coverage: enabled")
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

# ────────────────────────────────────────────────────────────────────────────
# Subdirectories
# ────────────────────────────────────────────────────────────────────────────
add_subdirectory(src)
