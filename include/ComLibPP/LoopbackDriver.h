//
// Created by didal on 25/08/2025.
//

#ifndef COMLIBPP_LOOPBACKDRIVER_H
#define COMLIBPP_LOOPBACKDRIVER_H

#include "ISerialDriver.hpp"

namespace wincom
{
    class LoopbackDriver final : public ISerialDriver
    {
    public:
        LoopbackDriver(std::string portName, const SerialSettings &settings, const TimeoutPolicy &timeoutPolicy);
        ~LoopbackDriver() override;
        void open(std::string portName, const SerialSettings &settings, const TimeoutPolicy &timeoutPolicy) override;
        [[nodiscard]] bool isOpen() const override;
        void close() override;
        void setLineCoding(const SerialSettings &settings) override;
        void setTimeouts(const TimeoutPolicy& policy) override;
        std::size_t readSome(uint8_t* dst, std::size_t maxBytes, std::chrono::milliseconds timeout) override;
        std::size_t writeSome(const uint8_t* src, std::size_t n, std::chrono::milliseconds) override;

        [[nodiscard]] std::size_t bytesAvailable() const override;
        void cancelIo() override;

    private:
        [[maybe_unused]]  void throwLastError_(const char* what);
        const TimeoutPolicy& getTimeoutPolicy() const override;
        const SerialSettings& getSerialSettings() const override;

    private:
        std::vector<uint8_t> m_Buffer;
        bool m_IsOpen{false};
        TimeoutPolicy   m_Policy {};
        SerialSettings  m_Settings {};
    };
}

#endif //COMLIBPP_LOOPBACKDRIVER_H