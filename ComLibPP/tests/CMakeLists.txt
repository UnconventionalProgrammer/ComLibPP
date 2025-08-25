include(FetchContent)
FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(catch2)

add_executable(ComLibPP_tests test_main.cpp LoopbackDriver.cpp)
target_link_libraries(ComLibPP_tests PRIVATE ComLibPP Catch2::Catch2WithMain)
add_test(NAME ComLibPP_tests COMMAND ComLibPP_tests)