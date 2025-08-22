cmake_minimum_required(VERSION 3.21)
project(WinComLibPP VERSION 1.0.0 LANGUAGES CXX)

option(WINCOMLIBPP_BUILD_SHARED "Build shared library" OFF)
option(WINCOMLIBPP_BUILD_SHARED  "Build tests" OFF)
option(WINCOMLIBPP_BUILD_SHARED "Treat warnings as errors" ON)

# Library
set(WINCOMLIBPP_HEADERS
        include/WinComLibPP/WinComLibPP.hpp
        include/WinComLibPP/export.hpp
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
#add_executable(example_minimal examples/minimal.cpp)
#target_link_libraries(example_minimal PRIVATE jnz::WinComLibPP)

# Tests
#include(CTest)
#if (MYLIB_BUILD_TESTS)
#    add_subdirectory(tests)
#endif()

