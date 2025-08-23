#include <chrono>
#include <format>
#include "WinComLibPP/Win32SerialDriver.hpp"
#include "WinComLibPP/WinComLibPP.hpp"

using namespace std::chrono_literals;

int main()
{
#ifdef _WIN32
    wincom::Win32Serial driver("COM1",
        {115200, 8, wincom::Win32Serial::Parity::none, wincom::Win32Serial::StopBits::one},
        {wincom::Win32Serial::TimeoutMode::finite, 100ms, 200ms});

    wincom::SerialStream<wincom::Win32Serial> stream(driver);

    wincom::Win32Serial driver2("COM2",
        {115200, 8, wincom::Win32Serial::Parity::none, wincom::Win32Serial::StopBits::one},
        {wincom::Win32Serial::TimeoutMode::finite, 100ms, 200ms});

    wincom::SerialStream<wincom::Win32Serial> stream2(driver2);

    stream << "Bonjour" << std::flush;
    stream2 << "Hello" << std::flush;

    std::string line;
    while (std::getline(stream, line) || std::getline(stream2, line))
    {
        if (line == "Bonjour")
            std::cout << "Salute\n";

        if (line == "Hello")
            std::cout << "Hi\n";
    }
#endif
}