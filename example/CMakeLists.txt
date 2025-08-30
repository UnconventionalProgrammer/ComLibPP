# build every .cpp under example/ as an example exe
file(GLOB EXAMPLE_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(src ${EXAMPLE_SRCS})
    get_filename_component(name "${src}" NAME_WE)
    add_executable("example_${name}" "${src}")
    target_link_libraries("example_${name}" PRIVATE ucpgr::ComLibPP)
endforeach()
