cmake_minimum_required(VERSION 3.20)

# ---- Project ----
project(ComLibPP
        VERSION 0.1.0
        LANGUAGES CXX)

# ---- Options ----
option(COMLIBPP_INSTALL "Install targets" ON)

# ---- Interface target (always exists) ----
# Consumers should link against ucpgr::comlibpp on all platforms.
add_library(ucpgr_comlibpp INTERFACE)

# Public headers live here
target_include_directories(ucpgr_comlibpp
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

# Require a sane C++ level for consumers
target_compile_features(ucpgr_comlibpp INTERFACE cxx_std_20)

# Provide the expected namespaced alias
add_library(ucpgr::comlibpp ALIAS ucpgr_comlibpp)

# ---- Windows implementation (optional, auto-wired) ----
if(WIN32)
    # Collect sources (list them explicitly in real projects; GLOB keeps this short)
    file(GLOB COMLIBPP_SRC CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

    if(COMLIBPP_SRC)
        add_library(comlibpp_impl STATIC ${COMLIBPP_SRC})
        target_compile_features(comlibpp_impl PRIVATE cxx_std_20)
        target_include_directories(comlibpp_impl
                PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include)

        # Link Win32 dependencies you actually use (adjust if needed)
        target_link_libraries(comlibpp_impl
                PRIVATE
                setupapi)

        # Make the interface pull in the impl on Windows automatically
        target_link_libraries(ucpgr_comlibpp
                INTERFACE
                comlibpp_impl)
    endif()
endif()