include(FetchContent)
FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(catch2)

file(GLOB TEST_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(comlibpp_tests ${TEST_SRCS})
target_link_libraries(comlibpp_tests PRIVATE ucpgr::ComLibPP Catch2::Catch2WithMain)
add_test(NAME comlibpp_tests COMMAND comlibpp_tests)
