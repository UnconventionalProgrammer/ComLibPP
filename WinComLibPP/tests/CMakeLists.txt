include(FetchContent)
FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(catch2)

add_executable(WinComLibPP_tests test_main.cpp LoopbackDriver.cpp)
target_link_libraries(WinComLibPP_tests PRIVATE WinComLibPP Catch2::Catch2WithMain)
add_test(NAME WinComLibPP_tests COMMAND WinComLibPP_tests)