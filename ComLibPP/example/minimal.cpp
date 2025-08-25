#include <chrono>
#include <format>
#include "COMLIBPP/Win32SerialDriver.hpp"
#include "COMLIBPP/COMLIBPP.hpp"
test
using namespace std::chrono_literals;

int main()
{
    wincom::SerialStream<wincom::Win32Serial> stream("COM1", wincom::Win32Serial::SerialSettings{115200, 8, wincom::Win32Serial::Parity::none, wincom::Win32Serial::StopBits::one}, wincom::Win32Serial::TimeoutPolicy{wincom::Win32Serial::TimeoutMode::finite, 100ms, 200ms});
    wincom::SerialStream<wincom::Win32Serial> stream2("COM2", wincom::Win32Serial::SerialSettings{115200, 8, wincom::Win32Serial::Parity::none, wincom::Win32Serial::StopBits::one}, wincom::Win32Serial::TimeoutPolicy{wincom::Win32Serial::TimeoutMode::finite, 100ms, 200ms});

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