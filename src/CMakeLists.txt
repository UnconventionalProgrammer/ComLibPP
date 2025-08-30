# Sources (adjust names if yours differ)
set(COMLIBPP_PUBLIC_HEADERS
        ${PROJECT_SOURCE_DIR}/include/ComLibPP/export.hpp
        ${PROJECT_SOURCE_DIR}/include/ComLibPP/ISerialDriver.hpp
        ${PROJECT_SOURCE_DIR}/include/ComLibPP/Win32SerialDriver.hpp   # only exists on Windows
        ${PROJECT_SOURCE_DIR}/include/ComLibPP/ComLibPP.hpp
        ${PROJECT_SOURCE_DIR}/include/ComLibPP/LoopbackDriver.h
)

set(COMLIBPP_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/ComLibPP.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LoopbackDriver.cpp
)

# Library kind
if (COMLIBPP_BUILD_SHARED)
    add_library(ComLibPP SHARED ${COMLIBPP_SOURCES} ${COMLIBPP_PUBLIC_HEADERS})
    target_compile_definitions(ComLibPP PRIVATE COMLIBPP_BUILD_SHARED)
else()
    add_library(ComLibPP STATIC ${COMLIBPP_SOURCES} ${COMLIBPP_PUBLIC_HEADERS})
endif()

add_library(ucpgr::ComLibPP ALIAS ComLibPP)

target_include_directories(ComLibPP
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(ComLibPP PUBLIC cxx_std_20)

# Warnings
if (MSVC)
    target_compile_options(ComLibPP PRIVATE /W4)
else()
    target_compile_options(ComLibPP PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install
install(TARGETS ComLibPP
        EXPORT ComLibPPTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
