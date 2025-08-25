cmake_minimum_required(VERSION 3.21)
project(WinComLibPP VERSION 0.1.0 LANGUAGES CXX)

option(WINCOMLIBPP_BUILD_SHARED        "Build shared library"          OFF)
option(WINCOMLIBPP_BUILD_TESTS         "Build tests"                   ON)
option(WINCOMLIBPP_WARNINGS_AS_ERRORS  "Treat warnings as errors"      ON)


# Figure out if building static or shared
set(LIB_KIND "static")
if (BUILD_SHARED_LIBS OR WINCOMLIBPP_BUILD_SHARED)
    set(LIB_KIND "shared")
endif()

# Handle multi-config generators (Visual Studio, Xcode, etc.)
foreach(OUTPUTCONFIG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)

    # Library outputs
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}
            ${CMAKE_SOURCE_DIR}/bin/${LIB_KIND}/${OUTPUTCONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}
            ${CMAKE_SOURCE_DIR}/bin/${LIB_KIND}/${OUTPUTCONFIG})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}
            ${CMAKE_SOURCE_DIR}/bin/${LIB_KIND}/${OUTPUTCONFIG})
endforeach()

# Helper: put an example exe in bin/example/<config>
function(set_example_output target)
    foreach(OUTPUTCONFIG Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
        set_target_properties(${target} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}
                ${CMAKE_SOURCE_DIR}/bin/example/${OUTPUTCONFIG}
        )
    endforeach()
endfunction()

# Helper: put a test exe in bin/test/<config>
function(set_test_output target)
    foreach(OUTPUTCONFIG Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
        set_target_properties(${target} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}
                ${CMAKE_SOURCE_DIR}/bin/test/${OUTPUTCONFIG}
        )
    endforeach()
endfunction()


# Library
set(WINCOMLIBPP_HEADERS
        include/WinComLibPP/WinComLibPP.hpp
        include/WinComLibPP/export.hpp
        include/WinComLibPP/ISerialDriver.hpp
        include/WinComLibPP/Win32SerialDriver.hpp
)

if (WINCOMLIBPP_BUILD_SHARED)
    add_library(WinComLibPP SHARED src/WinComLibPP.cpp ${WINCOMLIBPP_HEADERS})
    target_compile_definitions(WinComLibPP PRIVATE WINCOMLIBPP_BUILD_SHARED)
else()
    add_library(WinComLibPP STATIC src/WinComLibPP.cpp ${WINCOMLIBPP_HEADERS})
endif()

# Public interface
add_library(jnz::WinComLibPP ALIAS WinComLibPP)
target_include_directories(WinComLibPP
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(WinComLibPP PUBLIC cxx_std_20)

# Reasonable warnings
if (MSVC)
    target_compile_options(WinComLibPP PRIVATE /W4)
else()
    target_compile_options(WinComLibPP PRIVATE -Wall -Wextra -Wpedantic)
endif()
if (WINCOMLIBPP_WARNINGS_AS_ERRORS)
    if (MSVC)
        target_compile_options(WinComLibPP PRIVATE /WX)
    else()
        target_compile_options(WinComLibPP PRIVATE -Werror)
    endif()
endif()

# Version + package config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/WinComLibPPConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        cmake/WinComLibPPConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/WinComLibPPConfig.cmake"
        INSTALL_DESTINATION lib/cmake/WinComLibPP
)

# Install rules
include(GNUInstallDirs)
install(TARGETS WinComLibPP
        EXPORT WinComLibPPTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT WinComLibPPTargets
        NAMESPACE jnz::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/WinComLibPP
)
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/WinComLibPPConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/WinComLibPPConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/WinComLibPP
)

# Examples
add_executable(example_minimal example/minimal.cpp)
target_link_libraries(example_minimal PRIVATE jnz::WinComLibPP)
set_example_output(example_minimal)

# Tests
include(CTest)
if (WINCOMLIBPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()
