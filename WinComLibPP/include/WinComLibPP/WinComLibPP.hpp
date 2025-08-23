#ifndef WINCOMLIBPP_HPP
#define WINCOMLIBPP_HPP


#include <cstdint>
#include <cstring>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <streambuf>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#include "ISerialDriver.hpp"
#include "export.hpp"

namespace wincom
{

template <class Driver>
class SerialStreamBuf final : public std::streambuf
{
public:
    explicit SerialStreamBuf(Driver& driver,
                             const Driver::TimeoutPolicy &policy = {},
                             const std::size_t getBufSize = 4096,
                             const std::size_t putBufSize = 4096)
        : m_Driver(driver),
          m_Policy{policy},
          m_InBuf(getBufSize),
          m_OutBuf(putBufSize)
    {
        // empty get area
        setg(reinterpret_cast<char*>(m_InBuf.data()),
             reinterpret_cast<char*>(m_InBuf.data()),
             reinterpret_cast<char*>(m_InBuf.data()));

        // empty put area
        setp(reinterpret_cast<char*>(m_OutBuf.data()),
             reinterpret_cast<char*>(m_OutBuf.data() + m_OutBuf.size()));
    }

    ~SerialStreamBuf() override
    {
        SerialStreamBuf::sync();
    }

    SerialStreamBuf(const SerialStreamBuf&) = delete;
    SerialStreamBuf& operator=(const SerialStreamBuf&) = delete;

protected:
    // refill get area
    int_type underflow() override
    {
        if (gptr() < egptr())
        {
            return traits_type::to_int_type(*gptr());
        }

        // choose timeout per policy
        auto tmo = timeoutForRead_();

        std::size_t got = m_Driver.readSome(m_InBuf.data(), m_InBuf.size(), tmo);
        if (got == 0)
        {
            // timeout / no data (not a fatal EOF)
            return traits_type::eof();
        }

        setg(reinterpret_cast<char*>(m_InBuf.data()),
             reinterpret_cast<char*>(m_InBuf.data()),
             reinterpret_cast<char*>(m_InBuf.data() + got));
        return traits_type::to_int_type(*gptr());
    }

    // flush put area
    int sync() override
    {
        return flushOut_() ? 0 : -1;
    }

    // write one char (or flush)
    int_type overflow(int_type ch) override
    {
        if (!traits_type::eq_int_type(ch, traits_type::eof()))
        {
            *pptr() = traits_type::to_char_type(ch);
            pbump(1);
        }
        return flushOut_() ? traits_type::not_eof(ch) : traits_type::eof();
    }

    // write block
    std::streamsize xsputn(const char* s, std::streamsize n) override
    {
        std::streamsize total = 0;
        while (n > 0)
        {
            auto room = epptr() - pptr();
            if (room == 0)
            {
                if (!flushOut_())
                {
                    break;
                }
                room = epptr() - pptr();
            }

            auto chunk = static_cast<std::streamsize>(std::min<std::ptrdiff_t>(room, n));
            std::memcpy(pptr(), s, static_cast<std::size_t>(chunk));
            pbump(static_cast<int>(chunk));
            s += chunk;
            n -= chunk;
            total += chunk;

            if ((epptr() - pptr()) < 64)
            {
                if (!flushOut_())
                {
                    break;
                }
            }
        }
        return total;
    }

    // read block
    std::streamsize xsgetn(char* s, std::streamsize n) override
    {
        std::streamsize total = 0;

        while (n > 0)
        {
            if (gptr() == egptr())
            {
                if (traits_type::eq_int_type(underflow(), traits_type::eof()))
                {
                    break;
                }
            }

            auto avail = egptr() - gptr();
            auto take  = static_cast<std::streamsize>(std::min<std::ptrdiff_t>(avail, n));
            std::memcpy(s, gptr(), static_cast<std::size_t>(take));
            gbump(static_cast<int>(take));
            s += take;
            n -= take;
            total += take;
        }

        return total;
    }

    // serial ports are not seekable
    pos_type seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode) override
    {
        return {static_cast<off_type>(-1)};
    }

    pos_type seekpos(pos_type, std::ios_base::openmode) override
    {
        return {static_cast<off_type>(-1)};
    }

private:
    bool flushOut_()
    {
        auto n = static_cast<std::size_t>(pptr() - pbase());
        if (n == 0)
        {
            return true;
        }

        auto tmo = timeoutForWrite_();

        std::size_t written = 0;
        while (written < n)
        {
            std::size_t w = m_Driver.writeSome(
                reinterpret_cast<const uint8_t*>(pbase()) + written,
                n - written,
                tmo);

            if (w == 0)
            {
                // timed out (non-fatal) — leave remaining bytes in buffer
                // You can choose to treat this as failure if you prefer.
                break;
            }
            written += w;
        }

        // shift remaining (if any) to beginning
        const auto remaining = n - written;
        if (remaining > 0)
        {
            std::memmove(pbase(),
                         reinterpret_cast<const uint8_t*>(pbase()) + written,
                         remaining);
        }

        setp(pbase(), epptr());
        pbump(static_cast<int>(remaining));
        return written > 0 || remaining == 0;
    }

    [[nodiscard]] std::chrono::milliseconds timeoutForRead_() const
    {
        switch (m_Policy.mode)
        {
            case Driver::TimeoutMode::blocking:     return std::chrono::milliseconds{-1}; // sentinel: block forever
            case Driver::TimeoutMode::finite:       return m_Policy.readTimeout;
            case Driver::TimeoutMode::nonBlocking:  return std::chrono::milliseconds{0};
        }
        return m_Policy.readTimeout;
    }

    [[nodiscard]] std::chrono::milliseconds timeoutForWrite_() const
    {
        switch (m_Policy.mode)
        {
            case Driver::TimeoutMode::blocking:     return std::chrono::milliseconds{-1};
            case Driver::TimeoutMode::finite:       return m_Policy.writeTimeout;
            case Driver::TimeoutMode::nonBlocking:  return std::chrono::milliseconds{0};
        }
        return m_Policy.writeTimeout;
    }

private:
    Driver&                m_Driver;
    Driver::TimeoutPolicy  m_Policy;
    std::vector<uint8_t>   m_InBuf;
    std::vector<uint8_t>   m_OutBuf;
};

// ------------------------------
// iostream facade
// ------------------------------
template <class Driver>
class WINCOMLIBPP_API SerialStream final : public std::iostream
{
public:
    explicit SerialStream(Driver& driver,
                          Driver::TimeoutPolicy policy = {},
                          std::size_t getBufSize = 4096,
                          std::size_t putBufSize = 4096)
        : std::iostream(nullptr),
          m_Buf(driver, policy, getBufSize, putBufSize)
    {
        std::iostream::rdbuf(&m_Buf);
        unsetf(std::ios::skipws); // binary-friendly
    }

    SerialStreamBuf<Driver>* rdbuf()
    {
        return &m_Buf;
    }

private:
    SerialStreamBuf<Driver> m_Buf;
};

} // namespace wincom


#endif