//
// Created by didal on 25/08/2025.
//
#include <cstring>
#include "../include/ComLibPP/LoopbackDriver.h"

namespace ucpgr
{
    LoopbackDriver::LoopbackDriver(std::string portName, const SerialSettings &settings,
                                   const TimeoutPolicy &timeoutPolicy) : m_Policy(timeoutPolicy), m_Settings(settings)
    {
        this->open(std::move(portName), m_Settings, m_Policy);
    }

    LoopbackDriver::~LoopbackDriver()
    {
        LoopbackDriver::close();
    }

    void LoopbackDriver::open(std::string portName, const SerialSettings &settings,
                              const TimeoutPolicy &timeoutPolicy)
    {
        close();

        m_Settings = settings;
        m_Policy = timeoutPolicy;
        m_IsOpen = true;
    }

    [[nodiscard]] bool LoopbackDriver::isOpen() const
    {
        return m_IsOpen;
    }

    void LoopbackDriver::close()
    {
        if (isOpen())
        {
            m_Buffer.clear();
            m_IsOpen = false;
        }
    }

    void LoopbackDriver::setLineCoding(const SerialSettings &settings)
    {
        m_Settings = settings;
    }

    void LoopbackDriver::setTimeouts(const TimeoutPolicy &policy)
    {
        m_Policy = policy;
    }

    std::size_t LoopbackDriver::readSome(uint8_t *dst, std::size_t maxBytes,
                                         std::chrono::milliseconds timeout)
    {
        if (!isOpen())
            throw SerialError(std::make_error_code(std::errc::bad_file_descriptor), "readSome on closed port");

        std::size_t toRead = std::min(maxBytes, m_Buffer.size());
        if (toRead == 0)
            return 0;

        memcpy(dst, m_Buffer.data(), toRead);
        m_Buffer.erase(m_Buffer.begin(), m_Buffer.begin() + toRead);
        return toRead;
    }

    std::size_t LoopbackDriver::writeSome(const uint8_t *src, std::size_t n,
                                          std::chrono::milliseconds)
    {
        if (!isOpen())
            throw SerialError(std::make_error_code(std::errc::bad_file_descriptor), "writeSome on closed port");

        std::span data{src, n};
        m_Buffer.insert(m_Buffer.end(), data.begin(), data.end());

        return n;
    }

    [[nodiscard]] std::size_t LoopbackDriver::bytesAvailable() const
    {
        if (!isOpen())
            return 0;

        return m_Buffer.size();
    }

    void LoopbackDriver::cancelIo()
    {
        m_IsOpen = false;
    }

    void LoopbackDriver::throwLastError_(const char *what)
    {
        throw std::runtime_error(what);
    }

    const LoopbackDriver::TimeoutPolicy &LoopbackDriver::getTimeoutPolicy() const
    {
        return m_Policy;
    }

    const LoopbackDriver::SerialSettings &LoopbackDriver::getSerialSettings() const
    {
        return m_Settings;
    }
} // wincom
