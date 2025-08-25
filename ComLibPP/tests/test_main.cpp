#include <catch2/catch_all.hpp>

#include <string>
#include <array>
#include <vector>
#include <chrono>
#include <cstdint>
#include <iostream>


#include "LoopbackDriver.h"
#include "catch2/catch_test_macros.hpp"
#include "ComLibPP.hpp"

using namespace std::chrono_literals;


using Driver = wincom::LoopbackDriver;
static constexpr const char* kPort = "LOOPBACK";

static Driver::SerialSettings makeSettings()
{
    return Driver::SerialSettings{
        115200,
        8,
        Driver::Parity::none,
        Driver::StopBits::one
    };
}

static Driver::TimeoutPolicy makePolicy()
{
    return Driver::TimeoutPolicy{
        Driver::TimeoutMode::finite,
        100ms,   // read timeout
        200ms    // write timeout
    };
}

TEST_CASE("Text round-trip via SerialStream", "[serial][text]")
{
    wincom::SerialStream<wincom::LoopbackDriver> stream{std::string{kPort}, makeSettings(), makePolicy()};

    const std::string msg = "Hello, world!";
    SECTION("write with newline, read with getline")
    {
        stream << msg << '\n' << std::flush;

        std::string line;
        REQUIRE(std::getline(stream, line));
        REQUIRE(line == msg);
    }
}

TEST_CASE("Binary round-trip via SerialStream", "[serial][binary]")
{
    wincom::SerialStream<wincom::LoopbackDriver> stream{std::string{kPort}, makeSettings(), makePolicy()};

    const std::array<uint8_t, 6> outBuf{ 0x00, 0x01, 0x02, 0xFE, 0xFF, 0x7F };

    SECTION("write/read exact N bytes")
    {
        stream.write(reinterpret_cast<const char*>(outBuf.data()),
                     static_cast<std::streamsize>(outBuf.size()));
        stream.flush();

        std::array<uint8_t, outBuf.size()> inBuf{};
        stream.read(reinterpret_cast<char*>(inBuf.data()),
                    static_cast<std::streamsize>(inBuf.size()));
        auto got = static_cast<std::size_t>(stream.gcount());

        REQUIRE(got == outBuf.size());
        REQUIRE(std::equal(inBuf.begin(), inBuf.end(), outBuf.begin()));
    }
}

TEST_CASE("Multiple writes preserve ordering", "[serial][ordering]")
{
    wincom::SerialStream<wincom::LoopbackDriver> stream{std::string{kPort}, makeSettings(), makePolicy()};

    SECTION("two lines written -> two lines read in order")
    {
        stream << "part1" << '\n';
        stream << "part2" << '\n';
        stream.flush();

        std::string l1, l2;
        REQUIRE(std::getline(stream, l1));
        REQUIRE(std::getline(stream, l2));
        REQUIRE(l1 == "part1");
        REQUIRE(l2 == "part2");
    }
}
