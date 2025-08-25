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

# C++ standard (don’t fight the toolchain with --std flags)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(COMLIBPP_PUBLIC_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ucpgr/export.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ucpgr/ComLibPP.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ucpgr/ISerialDriver.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ucpgr/Win32SerialDriver.hpp
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
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ComLibPPConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ComLibPPConfig.cmake.in
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
if (COMLIBPP_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/example/CMakeLists.txt")
    add_subdirectory(example)
endif()

# -----------------------------------------
# Tests (if present)
# -----------------------------------------
if (COMLIBPP_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()
