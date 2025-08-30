#include <chrono>
#include <format>
#include <ComLibPP/Win32SerialDriver.hpp>
#include <ComLibPP/ComLibPP.hpp>

using namespace std::chrono_literals;

int main()
{
    ucpgr::SerialStream<ucpgr::Win32Serial> stream("COM1");
    ucpgr::SerialStream<ucpgr::Win32Serial> stream2("COM2");

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