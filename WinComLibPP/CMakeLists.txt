cmake_minimum_required(VERSION 3.21)

# -----------------------------------------
# Project
# -----------------------------------------
project(WinComLibPP VERSION 0.1.0 LANGUAGES CXX)

# Options
option(WINCOMLIBPP_BUILD_SHARED   "Build WinComLibPP as a shared library" OFF)
option(WINCOMLIBPP_BUILD_EXAMPLES "Build examples"                         ON)
option(WINCOMLIBPP_BUILD_TESTS    "Build tests (with CTest)"               ON)
option(WINCOMLIBPP_WERROR         "Treat warnings as errors"               ON)

# C++ standard (don’t fight the toolchain with --std flags)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------
# Output directory scheme
# -----------------------------------------
# Decide lib kind bucket
set(_LIB_KIND "static")
if (WINCOMLIBPP_BUILD_SHARED OR BUILD_SHARED_LIBS)
    set(_LIB_KIND "shared")
endif()

# Single-config (Makefiles/Ninja) default dirs
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/WinComLibPP/bin/${_LIB_KIND})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/WinComLibPP/bin/${_LIB_KIND})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/WinComLibPP/bin/${_LIB_KIND})

# Multi-config (MSVC/Xcode) per-config dirs
foreach(_cfg Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${_cfg} _CFG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_CFG} ${CMAKE_BINARY_DIR}/WinComLibPP/bin/${_LIB_KIND}/${_cfg})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_CFG} ${CMAKE_BINARY_DIR}/WinComLibPP/bin/${_LIB_KIND}/${_cfg})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_CFG} ${CMAKE_BINARY_DIR}/WinComLibPP/bin/${_LIB_KIND}/${_cfg})
endforeach()

# Helper to route example binaries → WinComLibPP/example/<config>
function(wincom_set_example_output target)
    # single-config
    set_target_properties(${target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/WinComLibPP/example)
    # multi-config
    foreach(_cfg Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER ${_cfg} _CFG)
        set_target_properties(${target} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY_${_CFG} ${CMAKE_BINARY_DIR}/WinComLibPP/example/${_cfg})
    endforeach()
endfunction()

# Helper to route test binaries → WinComLibPP/tests/  (no config subdir)
function(wincom_set_test_output target)
    # single-config
    set_target_properties(${target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/WinComLibPP/tests)
    # multi-config
    foreach(_cfg Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER ${_cfg} _CFG)
        set_target_properties(${target} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY_${_CFG} ${CMAKE_BINARY_DIR}/WinComLibPP/tests)
    endforeach()
endfunction()

# -----------------------------------------
# Library target
# -----------------------------------------
# Collect sources (swap to explicit list if you prefer; glob is OK for now)
file(GLOB_RECURSE WINCOMLIBPP_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(WINCOMLIBPP_PUBLIC_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/wincom/export.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/wincom/serial_stream.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/wincom/ISerialDriver.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/wincom/Win32Serial.hpp
)

if (WINCOMLIBPP_BUILD_SHARED)
    add_library(WinComLibPP SHARED ${WINCOMLIBPP_SOURCES})
    target_compile_definitions(WinComLibPP PRIVATE WINCOMLIBPP_BUILD_SHARED)
else()
    add_library(WinComLibPP STATIC ${WINCOMLIBPP_SOURCES})
endif()
add_library(wincom::WinComLibPP ALIAS WinComLibPP)

target_include_directories(WinComLibPP
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(WinComLibPP PUBLIC cxx_std_20)

# Warnings
if (MSVC)
    target_compile_options(WinComLibPP PRIVATE /W4)
else()
    target_compile_options(WinComLibPP PRIVATE -Wall -Wextra -Wpedantic)
endif()
if (WINCOMLIBPP_WERROR)
    if (MSVC)
        target_compile_options(WinComLibPP PRIVATE /WX)
    else()
        target_compile_options(WinComLibPP PRIVATE -Werror)
    endif()
endif()

# -----------------------------------------
# Install + package config (optional but standard)
# -----------------------------------------
include(GNUInstallDirs)
install(TARGETS WinComLibPP
        EXPORT WinComLibPPTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/WinComLibPPConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/WinComLibPPConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/WinComLibPPConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/WinComLibPP
)
install(EXPORT WinComLibPPTargets
        NAMESPACE wincom::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/WinComLibPP
)
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/WinComLibPPConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/WinComLibPPConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/WinComLibPP
)

# -----------------------------------------
# Examples (if present)
# -----------------------------------------
if (WINCOMLIBPP_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()

# -----------------------------------------
# Tests (if present)
# -----------------------------------------
if (WINCOMLIBPP_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()
