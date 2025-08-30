cmake_minimum_required(VERSION 3.21)
project(ComLibPP VERSION 0.1.0 LANGUAGES CXX)

# Options (default OFF so FetchContent users don't get surprise targets)
option(COMLIBPP_BUILD_SHARED   "Build ComLibPP as a shared lib"       OFF)
option(COMLIBPP_BUILD_EXAMPLES "Build examples"                        OFF)
option(COMLIBPP_BUILD_TESTS    "Build tests (CTest + Catch2)"          OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Be quiet when we're a subproject (FetchContent/add_subdirectory)
if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    message(STATUS "Configuring ComLibPP ${PROJECT_VERSION}")
endif()

# Library
add_subdirectory(src)

# Examples & Tests only if requested (and if the folders exist)
if (COMLIBPP_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/example/CMakeLists.txt")
    add_subdirectory(example)
endif()

if (COMLIBPP_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()

# Install & package-config (so users can also do find_package(ComLibPP))
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/ComLibPPConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ComLibPPConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ComLibPPConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ComLibPP"
)

install(EXPORT ComLibPPTargets
        NAMESPACE ucpgr::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ComLibPP"
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ComLibPPConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ComLibPPConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ComLibPP"
)

# Optional: export a build-tree package for 'find_package' without installing
export(EXPORT ComLibPPTargets
        NAMESPACE ucpgr::
        FILE "${CMAKE_CURRENT_BINARY_DIR}/ComLibPPTargets.cmake")
