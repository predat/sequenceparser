# ────────────────────────────────────────────────────────────────────────────
# Sources
# ────────────────────────────────────────────────────────────────────────────
file(GLOB_RECURSE SEQUENCEPARSER_SRC_FILES
    "sequenceParser/*.cpp"
    "sequenceParser/*.hpp"
)

# ────────────────────────────────────────────────────────────────────────────
# Helper macro: apply common settings to a sequenceParser target
# ────────────────────────────────────────────────────────────────────────────
macro(sequenceparser_configure_target _target)
    target_include_directories(${_target} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    target_compile_features(${_target} PUBLIC cxx_std_17)
    set_target_properties(${_target} PROPERTIES
        OUTPUT_NAME     sequenceParser
        LINKER_LANGUAGE CXX
    )
endmacro()

# Define the name of our export targets set.
# This export targets set will contain all the targets we want to expose to the outside world.
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")

# ────────────────────────────────────────────────────────────────────────────
# Static library
# ────────────────────────────────────────────────────────────────────────────
if(BUILD_STATIC_LIBS)
    add_library(sequenceparser-static STATIC ${SEQUENCEPARSER_SRC_FILES})
    sequenceparser_configure_target(sequenceparser-static)
endif()

# ────────────────────────────────────────────────────────────────────────────
# Shared library
# ────────────────────────────────────────────────────────────────────────────
if(BUILD_SHARED_LIBS)
    add_library(sequenceparser-shared SHARED ${SEQUENCEPARSER_SRC_FILES})
    sequenceparser_configure_target(sequenceparser-shared)
    set_target_properties(sequenceparser-shared PROPERTIES
        SOVERSION ${SEQUENCEPARSER_VERSION_MAJOR}
        VERSION   ${SEQUENCEPARSER_VERSION}
        # The code has no __declspec(dllexport) annotations, so on Windows MSVC
        # would export nothing and produce no import .lib — anything linking
        # against it (the SWIG modules) then fails with LNK1181. Auto-export
        # every symbol so the import library is generated. No-op elsewhere.
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
    # WINDOWS_EXPORT_ALL_SYMBOLS builds the export .def by scanning the object
    # files' COFF symbol tables, but with IPO/LTO (/GL) the objects hold an
    # intermediate representation instead — the scan crashes (access violation).
    # The two are incompatible, so turn IPO off for this target on Windows only;
    # everything else keeps the global CMAKE_INTERPROCEDURAL_OPTIMIZATION.
    if(WIN32)
        set_target_properties(sequenceparser-shared PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION OFF)
    endif()
endif()

# ────────────────────────────────────────────────────────────────────────────
# Install: headers + libraries
# ────────────────────────────────────────────────────────────────────────────
# Only the public headers needed to use libsequenceParser from C++.
# The detail/ headers and utils.hpp are implementation details (never included
# by a public header — Sequence.hpp only forward-declares detail::FileNumbers),
# so they are intentionally not installed.
install(
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/sequenceParser/sequenceParser.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/sequenceParser/common.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/sequenceParser/FrameRange.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/sequenceParser/Sequence.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/sequenceParser/Sequence.tcc"
        "${CMAKE_CURRENT_SOURCE_DIR}/sequenceParser/Item.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/sequenceParser/ItemStat.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/sequenceParser/system.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/sequenceParser/filesystem.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/sequenceParser/detector.hpp"
    DESTINATION include/sequenceParser
)

# SWIG interface files: not C++ headers, only useful to regenerate Python/Java
# bindings against the installed library. Shipped under share/, not include/.
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/sequenceParser/"
    DESTINATION share/sequenceParser/swig
    FILES_MATCHING PATTERN "*.i"
)

if(BUILD_STATIC_LIBS)
    install(TARGETS sequenceparser-static
        EXPORT ${TARGETS_EXPORT_NAME}
        DESTINATION lib/ OPTIONAL
    )
endif()
if(BUILD_SHARED_LIBS)
    install(TARGETS sequenceparser-shared
        EXPORT ${TARGETS_EXPORT_NAME}
        DESTINATION lib/ OPTIONAL
    )
endif()

# ────────────────────────────────────────────────────────────────────────────
# Exporting targets
# ────────────────────────────────────────────────────────────────────────────

# With our targets properly defined, we can generate the configuration files
# to describe the targets we export.
# In this simple project, we only export our minicpp library, but we could have
# several of them.

include(CMakePackageConfigHelpers)

# Path to install CMake configuration files.
set(CONFIG_INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

set(CONFIG_FILE_TEMPLATE "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in")
set(CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake")
set(CONFIG_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake")

# Generate config file from template.
# Note: INSTALL_DESTINATION must match where the config file will be installed, but does not install it.
configure_package_config_file(
    ${CONFIG_FILE_TEMPLATE} ${CONFIG_FILE}
    INSTALL_DESTINATION ${CONFIG_INSTALL_DESTINATION}
)

# Generate a config version file to indicate backwards compatibility strategy.
write_basic_package_version_file(
    ${CONFIG_VERSION_FILE}
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Actually install the generated config and configVersion files.
install(
    FILES ${CONFIG_FILE} ${CONFIG_VERSION_FILE}
    #DESTINATION ${CONFIG_INSTALL_DESTINATION}
    DESTINATION lib/cmake
)

# Install our export target set next to the config files.
install(
    EXPORT ${TARGETS_EXPORT_NAME}
    FILE ${TARGETS_EXPORT_NAME}.cmake
    NAMESPACE ${PROJECT_NAME}::
    #DESTINATION ${CONFIG_INSTALL_DESTINATION}
    DESTINATION lib/cmake
)

# ────────────────────────────────────────────────────────────────────────────
# SWIG bindings (Python / Java) – optional
# ────────────────────────────────────────────────────────────────────────────
find_package(SWIG)
if(NOT SWIG_FOUND)
    message(STATUS "SWIG not found – skipping Python and Java bindings")
    return()
endif()

include(${SWIG_USE_FILE})

set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/sequenceParser")
set(SEQUENCEPARSER_BINDING_FILE "sequenceParser/sequenceParser.i")
set_source_files_properties(${SEQUENCEPARSER_BINDING_FILE} PROPERTIES
    CPLUSPLUS       ON
    SWIG_MODULE_NAME sequenceParser   # forces PyInit_sequenceParser (target name has a dash)
)

set(COMMON_SWIG_FLAGS -c++ -fcompact -small -doxygen -O -Werror)

# On 64-bit Windows std::ptrdiff_t (and thus Time) is `long long`, not `long`
# as on LP64 Linux/macOS. Tell SWIG so its Time typedef matches the real type
# (see src/sequenceParser/common.i), otherwise MSVC fails with error C2440.
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    list(APPEND COMMON_SWIG_FLAGS -DSEQUENCEPARSER_TIME_IS_LONGLONG)
endif()

# ── Python ──────────────────────────────────────────────────────────────────
# Use the modern FindPython3 module (replaces deprecated FindPythonLibs / CMP0148)
find_package(Python3 ${SEQUENCEPARSER_PYTHON_VERSION} COMPONENTS Interpreter Development)
if(Python3_Development_FOUND)
    set(_py_target sequenceParser)

    set(CMAKE_SWIG_FLAGS ${COMMON_SWIG_FLAGS})

    swig_add_library(${_py_target}
        TYPE        MODULE
        LANGUAGE    python
        SOURCES     ${SEQUENCEPARSER_BINDING_FILE}
    )

    target_include_directories(${_py_target} PRIVATE ${Python3_INCLUDE_DIRS})
    target_link_libraries(${_py_target} PRIVATE sequenceparser-shared Python3::Module)

    # The runtime module is `_sequenceParser.pyd`, but its import library would
    # default to `sequenceParser.lib` — the exact same path as the core shared
    # library's import lib (OUTPUT_NAME sequenceParser). On Windows that clobbers
    # it; name the module's implib after the module to avoid the collision.
    if(WIN32)
        set_target_properties(${_py_target} PROPERTIES ARCHIVE_OUTPUT_NAME _sequenceParser)
    endif()

    if(NOT APPLE AND NOT WIN32)
        set_target_properties(${_py_target} PROPERTIES
            SOVERSION ${SEQUENCEPARSER_VERSION_MAJOR}
            VERSION   ${SEQUENCEPARSER_VERSION}
        )
    endif()
    set_target_properties(${_py_target} PROPERTIES
        INSTALL_RPATH "$<IF:$<BOOL:${APPLE}>,@loader_path/../../..,$ORIGIN/../../..:$ORIGIN>"
    )

    # Derive major.minor string (e.g. "3.14") from the found Python3 version
    string(REGEX MATCH "^[0-9]+\\.[0-9]+" _python_version "${Python3_VERSION}")
    set(SEQUENCEPARSER_PYTHON_MODULE_OUTPUT_DIR
        "lib/python${_python_version}/site-packages/pySequenceParser/")

    install(
        FILES
            "${CMAKE_SWIG_OUTDIR}/sequenceParser.py"
            ${SEQUENCEPARSER_PYTHON_INIT_FILE}
        DESTINATION ${SEQUENCEPARSER_PYTHON_MODULE_OUTPUT_DIR}
        OPTIONAL
    )
    install(TARGETS ${_py_target}
        DESTINATION "${SEQUENCEPARSER_PYTHON_MODULE_OUTPUT_DIR}" OPTIONAL)
    install(CODE
        "file(WRITE ${CMAKE_INSTALL_PREFIX}/${SEQUENCEPARSER_PYTHON_MODULE_OUTPUT_DIR}/__init__.py)")
else()
    message(STATUS "Python3 Development not found – skipping Python binding")
endif()

# ── Java ─────────────────────────────────────────────────────────────────────
find_package(Java)
find_package(JNI)
if(JAVA_FOUND AND JNI_FOUND)
    include(UseJava)
    include_directories(${JAVA_INCLUDE_PATH} ${JNI_INCLUDE_DIRS})

    set(CMAKE_SWIG_FLAGS ${COMMON_SWIG_FLAGS} -package org.sequenceparser)

    swig_add_module(sequenceparser-java java ${SEQUENCEPARSER_BINDING_FILE})
    set(_java_target ${SWIG_MODULE_sequenceparser-java_REAL_NAME})

    set_target_properties(${_java_target} PROPERTIES OUTPUT_NAME sequenceParser-java)
    if(NOT APPLE AND NOT WIN32)
        set_target_properties(${_java_target} PROPERTIES
            SOVERSION ${SEQUENCEPARSER_VERSION_MAJOR}
            VERSION   ${SEQUENCEPARSER_VERSION}
        )
    endif()
    set_target_properties(${_java_target} PROPERTIES
        INSTALL_RPATH "$<IF:$<BOOL:${APPLE}>,@loader_path/..,${ORIGIN}/..:${ORIGIN}>"
    )
    swig_link_libraries(sequenceparser-java sequenceparser-shared)

    set(_java_src_dir  "${PROJECT_BINARY_DIR}/src/sequenceParser")
    set(_java_cls_dir  "${_java_src_dir}/target")
    set(_jar_path      "${PROJECT_BINARY_DIR}/src")
    set(_jar_name      "sequenceparser-${SEQUENCEPARSER_VERSION}.jar")
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${_java_cls_dir})

    add_custom_command(TARGET sequenceparser-java POST_BUILD
        COMMAND ${Java_JAVAC_EXECUTABLE}
                -d ${_java_cls_dir}
                "${_java_src_dir}/*.java"
        COMMAND ${Java_JAR_EXECUTABLE}
                cf ${_jar_name} -C ${_java_cls_dir} .
    )

    install(TARGETS ${_java_target} DESTINATION "lib/java" OPTIONAL)
    install(FILES "${_jar_path}/${_jar_name}" DESTINATION "share/java/" OPTIONAL)
else()
    message(STATUS "Java/JNI not found – skipping Java binding")
endif()
