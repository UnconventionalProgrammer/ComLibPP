//
// Created by didal on 23/08/2025.
//

#ifndef COMLIBPP_WIN32SERIALDRIVER_H
#define COMLIBPP_WIN32SERIALDRIVER_H


// =====================================================================
// Win32 driver (only compiled on Windows)
// =====================================================================
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <utility>
#include "ISerialDriver.hpp"
#include "export.hpp"

namespace ucpgr
{
class COMLIBPP_API Win32Serial final : public ISerialDriver
{
public:
    Win32Serial(std::string portName, const SerialSettings &settings = {}, const TimeoutPolicy &timeoutPolicy = {}) : m_Policy(timeoutPolicy), m_Settings(settings)
    {
        this->open(std::move(portName), m_Settings, m_Policy);
    }
    ~Win32Serial() override
    {
        Win32Serial::close();
    }

    void open(std::string portName, const SerialSettings &settings, const TimeoutPolicy &timeoutPolicy) override
    {
        close();

        m_Settings = settings;
        m_Policy = timeoutPolicy;

        if (portName.rfind(R"(\\.\)", 0) != 0)
        {
            portName = R"(\\.\)" + portName;
        }

        m_Handle = CreateFileA(portName.c_str(),
                               GENERIC_READ | GENERIC_WRITE,
                               0, nullptr, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                               nullptr);
        if (m_Handle == INVALID_HANDLE_VALUE)
        {
            throwLastError_("CreateFileA");
        }

        setLineCoding(m_Settings);
        setTimeouts(m_Policy);

        // clear buffers
        SetupComm(m_Handle, 1 << 16, 1 << 16);
        PurgeComm(m_Handle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    }

    [[nodiscard]] bool isOpen() const override
    {
        return m_Handle != INVALID_HANDLE_VALUE;
    }

    void close() override
    {
        if (isOpen())
        {
            cancelIo();
            CloseHandle(m_Handle);
            m_Handle = INVALID_HANDLE_VALUE;
        }
    }

    void setLineCoding(const SerialSettings &settings) override
    {
        if (!isOpen())
        {
            throw SerialError(std::make_error_code(std::errc::bad_file_descriptor), "setLineCoding on closed port");
        }

        DCB dcb{};
        dcb.DCBlength = sizeof dcb;
        if (!GetCommState(m_Handle, &dcb))
        {
            throwLastError_("GetCommState");
        }

        dcb.BaudRate = settings.baud;
        dcb.ByteSize = settings.dataBits;
        dcb.Parity   = toWinParity_(settings.parity);
        dcb.StopBits = toWinStopBits_(settings.stopBits);

        dcb.fOutxCtsFlow = FALSE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fDtrControl  = DTR_CONTROL_ENABLE;
        dcb.fRtsControl  = RTS_CONTROL_ENABLE;

        if (!SetCommState(m_Handle, &dcb))
        {
            throwLastError_("SetCommState");
        }
    }

    void setTimeouts(const TimeoutPolicy& policy) override
    {
        m_Policy = policy;

        // For overlapped I/O, COMMTIMEOUTS are not strictly required,
        // but we keep write timeout here as a safety net.
        COMMTIMEOUTS to{};
        switch (policy.mode)
        {
            case TimeoutMode::blocking:
            {
                to.ReadIntervalTimeout        = MAXDWORD;
                to.ReadTotalTimeoutMultiplier = 0;
                to.ReadTotalTimeoutConstant   = 0;
                break;
            }
            case TimeoutMode::finite:
            {
                auto ms = static_cast<DWORD>(policy.readTimeout.count());
                to.ReadIntervalTimeout        = ms;
                to.ReadTotalTimeoutMultiplier = 0;
                to.ReadTotalTimeoutConstant   = ms;
                break;
            }
            case TimeoutMode::nonBlocking:
            {
                to.ReadIntervalTimeout        = MAXDWORD;
                to.ReadTotalTimeoutMultiplier = 0;
                to.ReadTotalTimeoutConstant   = 0;
                break;
            }
        }
        to.WriteTotalTimeoutMultiplier = 0;
        to.WriteTotalTimeoutConstant   = static_cast<DWORD>(policy.writeTimeout.count());

        if (!SetCommTimeouts(m_Handle, &to))
        {
            throwLastError_("SetCommTimeouts");
        }
    }

    std::size_t readSome(uint8_t* dst, std::size_t maxBytes,
                         std::chrono::milliseconds timeout) override
    {
        if (!isOpen())
        {
            throw SerialError(std::make_error_code(std::errc::bad_file_descriptor), "readSome on closed port");
        }

        OVERLAPPED ov{};
        ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!ov.hEvent)
        {
            throwLastError_("CreateEventW");
        }

        DWORD got = 0;
        BOOL ok = ReadFile(m_Handle, dst, static_cast<DWORD>(maxBytes), &got, &ov);
        if (!ok && GetLastError() == ERROR_IO_PENDING)
        {
            const DWORD waitMs = toWaitMs_(timeout);
            if (const DWORD r = WaitForSingleObject(ov.hEvent, waitMs); r == WAIT_OBJECT_0)
            {
                if (!GetOverlappedResult(m_Handle, &ov, &got, FALSE))
                {
                    got = 0;
                }
            }
            else if (r == WAIT_TIMEOUT)
            {
                CancelIoEx(m_Handle, &ov);
                got = 0;
            }
            else
            {
                got = 0;
            }
        }
        else if (!ok)
        {
            got = 0;
        }

        CloseHandle(ov.hEvent);
        return static_cast<std::size_t>(got);
    }

    std::size_t writeSome(const uint8_t* src, std::size_t n,
                          std::chrono::milliseconds timeout) override
    {
        if (!isOpen())
        {
            throw SerialError(std::make_error_code(std::errc::bad_file_descriptor), "writeSome on closed port");
        }

        OVERLAPPED ov{};
        ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!ov.hEvent)
        {
            throwLastError_("CreateEventW");
        }

        DWORD written = 0;
        BOOL ok = WriteFile(m_Handle, src, static_cast<DWORD>(n), &written, &ov);
        if (!ok && GetLastError() == ERROR_IO_PENDING)
        {
            const DWORD waitMs = toWaitMs_(timeout);
            if (const DWORD r = WaitForSingleObject(ov.hEvent, waitMs); r == WAIT_OBJECT_0)
            {
                if (!GetOverlappedResult(m_Handle, &ov, &written, FALSE))
                {
                    written = 0;
                }
            }
            else if (r == WAIT_TIMEOUT)
            {
                CancelIoEx(m_Handle, &ov);
                written = 0;
            }
            else
            {
                written = 0;
            }
        }
        else if (!ok)
        {
            written = 0;
        }

        CloseHandle(ov.hEvent);
        return static_cast<std::size_t>(written);
    }

    [[nodiscard]] std::size_t bytesAvailable() const override
    {
        if (!isOpen())
        {
            return 0;
        }
        COMSTAT st{};
        DWORD errs{};
        if (!ClearCommError(m_Handle, &errs, &st))
        {
            return 0;
        }
        return static_cast<std::size_t>(st.cbInQue);
    }

    void cancelIo() override
    {
        if (isOpen())
        {
            CancelIoEx(m_Handle, nullptr);
        }
    }

private:
    static uint8_t toWinParity_(const Parity p)
    {
        switch (p)
        {
            case Parity::none: return NOPARITY;
            case Parity::odd:  return ODDPARITY;
            case Parity::even: return EVENPARITY;
            case Parity::mark: return MARKPARITY;
            case Parity::space:return SPACEPARITY;
        }
        return NOPARITY;
    }

    static uint8_t toWinStopBits_(const StopBits s)
    {
        switch (s)
        {
            case StopBits::one:         return ONESTOPBIT;
            case StopBits::onePointFive:return ONE5STOPBITS;
            case StopBits::two:         return TWOSTOPBITS;
        }
        return ONESTOPBIT;
    }

    static DWORD toWaitMs_(const std::chrono::milliseconds tmo)
    {
        if (tmo.count() < 0)  return INFINITE;     // our sentinel for "block forever"
        return static_cast<DWORD>(tmo.count());
    }

    static void throwLastError_(const char* what)
    {
        throw SerialError(std::error_code(static_cast<int>(GetLastError()), std::system_category()), what);
    }

    const TimeoutPolicy& getTimeoutPolicy() const override
    {
        return m_Policy;
    }
    const SerialSettings& getSerialSettings() const override
    {
        return m_Settings;
    }

private:
    HANDLE          m_Handle { INVALID_HANDLE_VALUE };
    TimeoutPolicy   m_Policy {};
    SerialSettings  m_Settings {};
};

} // namespace wincom
#endif // _WIN32

#endif //COMLIBPP_WIN32SERIALDRIVER_H