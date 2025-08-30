cmake_minimum_required(VERSION 3.21)


# -----------------------------------------
# Project
# -----------------------------------------
project(ComLibPP VERSION 0.1.0 LANGUAGES CXX)

# Options
option(COMLIBPP_BUILD_SHARED   "Build COMLIBPP as a shared library" OFF)
option(COMLIBPP_BUILD_EXAMPLES "Build examples"                         ON)
option(COMLIBPP_BUILD_TESTS    "Build tests (with CTest)"               ON)
option(COMLIBPP_WERROR         "Treat warnings as errors"               ON)
option(ENABLE_CPPCHECK "Run cppcheck during builds (when available)" ON)

find_program(CPPCHECK_EXE NAMES cppcheck DOC "Path to the cppcheck executable")

# C++ standard (don’t fight the toolchain with --std flags)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CPPCHECK_FOUND_AND_OK OFF)
if(ENABLE_CPPCHECK AND CPPCHECK_EXE)
    # Query version text, e.g. "Cppcheck 2.13"
    execute_process(
            COMMAND "${CPPCHECK_EXE}" --version
            OUTPUT_VARIABLE _cppcheck_ver_out
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # Extract just the version number
    string(REGEX REPLACE "^Cppcheck ([0-9]+\\.[0-9]+(\\.[0-9]+)?)$" "\\1"
            CPPCHECK_VERSION "${_cppcheck_ver_out}")

    if(CPPCHECK_VERSION STREQUAL _cppcheck_ver_out)
        # Fallback if regex didn’t match; don’t block on version
        message(STATUS "cppcheck found but version parse failed: '${_cppcheck_ver_out}'")
        set(CPPCHECK_FOUND_AND_OK ON)
    elseif(CPPCHECK_VERSION VERSION_LESS CPPCHECK_MIN_VERSION)
        message(WARNING
                "cppcheck ${CPPCHECK_VERSION} < required ${CPPCHECK_MIN_VERSION}; "
                "disabling cppcheck integration.")
    else()
        set(CPPCHECK_FOUND_AND_OK ON)
        message(STATUS "cppcheck ${CPPCHECK_VERSION} found: ${CPPCHECK_EXE}")
    endif()
elseif(ENABLE_CPPCHECK)
    message(STATUS "cppcheck not found; set CPPCHECK_EXE or install it to enable.")
endif()

# Common command-line (PER-FILE MODE — no --project here)
set(CPPCHECK_CMDLINE
        "${CPPCHECK_EXE};--enable=warning,performance,style,portability;--std=c++20;--inline-suppr;--suppress=missingIncludeSystem;--quiet;--template=gcc;--language=c++"
)

# Helper to attach cppcheck to YOUR targets only
function(enable_cppcheck_for target)
    if(CPPCHECK_FOUND_AND_OK)
        set_property(TARGET "${target}" PROPERTY CXX_CPPCHECK "${CPPCHECK_CMDLINE}")
    endif()
endfunction()


# -----------------------------------------
# Output directory scheme
# -----------------------------------------
# Decide lib kind bucket
set(_LIB_KIND "static")
if (COMLIBPP_BUILD_SHARED OR BUILD_SHARED_LIBS)
    set(_LIB_KIND "shared")
endif()

# Single-config (Makefiles/Ninja) default dirs
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ComLibPP/bin/${_LIB_KIND})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ComLibPP/bin/${_LIB_KIND})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ComLibPP/bin/${_LIB_KIND})

# Multi-config (MSVC/Xcode) per-config dirs
foreach(_cfg Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${_cfg} _CFG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_CFG} ${CMAKE_BINARY_DIR}/ComLibPP/bin/${_LIB_KIND}/${_cfg})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_CFG} ${CMAKE_BINARY_DIR}/ComLibPP/bin/${_LIB_KIND}/${_cfg})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_CFG} ${CMAKE_BINARY_DIR}/ComLibPP/bin/${_LIB_KIND}/${_cfg})
endforeach()

# Helper to route example binaries → COMLIBPP/example/<config>
function(wincom_set_example_output target)
    # single-config
    set_target_properties(${target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ComLibPP/example)
    # multi-config
    foreach(_cfg Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER ${_cfg} _CFG)
        set_target_properties(${target} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY_${_CFG} ${CMAKE_BINARY_DIR}/ComLibPP/example/${_cfg})
    endforeach()
endfunction()

# Helper to route test binaries → COMLIBPP/tests/  (no config subdir)
function(wincom_set_test_output target)
    # single-config
    set_target_properties(${target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ComLibPP/tests)
    # multi-config
    foreach(_cfg Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER ${_cfg} _CFG)
        set_target_properties(${target} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY_${_CFG} ${CMAKE_BINARY_DIR}/ComLibPP/tests)
    endforeach()
endfunction()

# -----------------------------------------
# Library target
# -----------------------------------------
# Collect sources (swap to explicit list if you prefer; glob is OK for now)
file(GLOB_RECURSE COMLIBPP_SOURCES CONFIGURE_DEPENDS
        ../src/*.cpp)
set(COMLIBPP_PUBLIC_HEADERS
        include/ucpgr/export.hpp
        include/ucpgr/ComLibPP.hpp
        include/ucpgr/ISerialDriver.hpp
        include/ucpgr/Win32SerialDriver.hpp
)

if (COMLIBPP_BUILD_SHARED)
    add_library(ComLibPP SHARED ${COMLIBPP_SOURCES})
    target_compile_definitions(ComLibPP PRIVATE COMLIBPP_BUILD_SHARED)
else()
    add_library(ComLibPP STATIC ${COMLIBPP_SOURCES} ${COMLIBPP_PUBLIC_HEADERS})
endif()
add_library(wincom::ComLibPP ALIAS ComLibPP)

target_include_directories(ComLibPP
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(ComLibPP PUBLIC cxx_std_20)

# Warnings
if (MSVC)
    target_compile_options(ComLibPP PRIVATE /W4)
else()
    target_compile_options(ComLibPP PRIVATE -Wall -Wextra -Wpedantic)
endif()
if (COMLIBPP_WERROR)
    if (MSVC)
        target_compile_options(ComLibPP PRIVATE /WX)
    else()
        target_compile_options(ComLibPP PRIVATE -Werror)
    endif()
endif()

# -----------------------------------------
# Install + package config (optional but standard)
# -----------------------------------------
include(GNUInstallDirs)
install(TARGETS ComLibPP
        EXPORT COMLIBPPTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ComLibPPConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
        cmake/ComLibPPConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ComLibPPConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ComLibPP
)
install(EXPORT COMLIBPPTargets
        NAMESPACE wincom::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ComLibPP
)
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ComLibPPConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ComLibPPConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ComLibPP
)

# -----------------------------------------
# Examples (if present)
# -----------------------------------------
if (COMLIBPP_BUILD_EXAMPLES AND EXISTS "example/CMakeLists.txt")
    add_subdirectory(example)
endif()

# -----------------------------------------
# Tests (if present)
# -----------------------------------------
if (COMLIBPP_BUILD_TESTS AND EXISTS "tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()


enable_cppcheck_for(ComLibPP)
enable_cppcheck_for(example_minimal)