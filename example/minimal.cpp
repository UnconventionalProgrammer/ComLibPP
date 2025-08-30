#include <chrono>
#include <format>
#include <ComLibPP/Win32SerialDriver.hpp>
#include <ComLibPP/ComLibPP.hpp>

using namespace std::chrono_literals;

int main()
{
    ucpgr::SerialStream<ucpgr::Win32Serial> stream("COM1", ucpgr::Win32Serial::SerialSettings{115200, 8, ucpgr::Win32Serial::Parity::none, ucpgr::Win32Serial::StopBits::one}, ucpgr::Win32Serial::TimeoutPolicy{ucpgr::Win32Serial::TimeoutMode::finite, 100ms, 200ms});
    ucpgr::SerialStream<ucpgr::Win32Serial> stream2("COM2", ucpgr::Win32Serial::SerialSettings{115200, 8, ucpgr::Win32Serial::Parity::none, ucpgr::Win32Serial::StopBits::one}, ucpgr::Win32Serial::TimeoutPolicy{ucpgr::Win32Serial::TimeoutMode::finite, 100ms, 200ms});

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
}