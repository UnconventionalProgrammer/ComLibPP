//
// Created by didal on 23/08/2025.
//

#ifndef COMLIBPP_ISERIALDRIVER_HPP
#define COMLIBPP_ISERIALDRIVER_HPP

#include <chrono>
#include <cstdint>
#include <system_error>
#include "export.hpp"

namespace ucpgr
{
    class COMLIBPP_API ISerialDriver
    {
    public:
        class SerialError final : public std::system_error
        {
        public:
            using std::system_error::system_error;
        };

        enum class Parity : uint8_t { none, odd, even, mark, space };
        enum class StopBits : uint8_t { one, onePointFive, two };

        enum class TimeoutMode : uint8_t { blocking, finite, nonBlocking };

        struct TimeoutPolicy
        {
            TimeoutMode mode = TimeoutMode::finite;
            std::chrono::milliseconds readTimeout  { 200 };
            std::chrono::milliseconds writeTimeout { 200 };
        };

        struct SerialSettings
        {
            uint32_t baud{115200};
            uint8_t dataBits{8};
            Parity parity{Parity::none};
            StopBits stopBits{StopBits::one};

        };

        virtual ~ISerialDriver() = default;

        // open/close
        virtual void open(std::string portName, const SerialSettings &settings, const TimeoutPolicy &timeoutPolicy) = 0;
        virtual void open(std::string portName, uint32_t baud) = 0;
        [[nodiscard]] virtual bool isOpen() const = 0;
        virtual void close() = 0;

        virtual void setLineCoding(const SerialSettings &settings) = 0;
        virtual void setTimeouts(const TimeoutPolicy& policy) = 0;
        // io
        // read up to maxBytes; returns bytes read (0 == timeout/non-blocking no data)
        virtual std::size_t readSome(uint8_t* dst, std::size_t maxBytes,
                                     std::chrono::milliseconds timeout) = 0;

        // write up to n; returns bytes written (0 == timeout/non-blocking cannot write)
        virtual std::size_t writeSome(const uint8_t* src, std::size_t n,
                                      std::chrono::milliseconds timeout) = 0;

        // hint: bytes available to read without blocking (best-effort)
        [[nodiscard]] virtual std::size_t bytesAvailable() const = 0;

        // cancel any blocking I/O (e.g., from another thread)
        virtual void cancelIo() = 0;

        const virtual TimeoutPolicy& getTimeoutPolicy() const = 0;
        const virtual SerialSettings& getSerialSettings() const = 0;

    private:
    };
}
#endif //COMLIBPP_ISERIALDRIVER_HPP