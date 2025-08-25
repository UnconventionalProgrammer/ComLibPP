// tests/test_serialstream_loopback.cpp
#include <catch2/catch_all.hpp>

#include <string>
#include <array>
#include <vector>
#include <chrono>
#include <cstdint>
#include <iostream>

// Adjust include paths to your project layout:

#include "LoopbackDriver.h"
#include "catch2/catch_test_macros.hpp"
#include "WinComLibPP/WinComLibPP.hpp"

using namespace std::chrono_literals;


using Driver = wincom::LoopbackDriver;
static constexpr const char* kPort = "LOOPBACK";

static wincom::SerialStream<Driver> makeStream()
{
    Driver::SerialSettings settings{
        115200,
        8,
        Driver::Parity::none,
        Driver::StopBits::one
    };

    Driver::TimeoutPolicy timeouts{
        Driver::TimeoutMode::finite,
        100ms,   // read timeout
        200ms    // write timeout
    };

    return wincom::SerialStream<Driver>(kPort, settings, timeouts);
}

TEST_CASE("Text round-trip via SerialStream", "[serial][text]")
{
    auto stream = makeStream();

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
    auto stream = makeStream();

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
    auto stream = makeStream();

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
