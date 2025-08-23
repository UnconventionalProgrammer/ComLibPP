#include <chrono>
#include "WinComLibPP/Win32SerialDriver.hpp"
#include "WinComLibPP/WinComLibPP.hpp"

using namespace std::chrono_literals;

int main()
{
#ifdef _WIN32
    wincom::Win32Serial driver;
    driver.open("COM3");
    driver.setLineCoding(115200, 8, wincom::Win32Serial::Parity::none, wincom::Win32Serial::StopBits::one);

    wincom::Win32Serial::TimeoutPolicy policy;
    policy.mode = wincom::Win32Serial::TimeoutMode::finite;
    policy.readTimeout  = 100ms;
    policy.writeTimeout = 200ms;

    wincom::SerialStream<wincom::Win32Serial> stream(driver, policy);

    // write text
    stream << "ATI\r\n" << std::flush;

    // read a line
    std::string line;
    if (std::getline(stream, line))
    {
        // process line
    }

    // binary read
    std::array<char, 32> buf{};
    stream.read(buf.data(), buf.size());
    auto got = stream.gcount();

    driver.close();
#endif
}