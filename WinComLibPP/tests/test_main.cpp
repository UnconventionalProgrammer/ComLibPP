#include <catch2/catch_all.hpp>
#include <WinComLibPP/WinComLibPP.hpp>

TEST_CASE("hello")
{
    REQUIRE(mylib::hello() == "Hello from mylib");
}