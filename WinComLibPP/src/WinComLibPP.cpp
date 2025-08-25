#include "WinComLibPP/WinComLibPP.hpp"

wincom::SerialStreamBuf::SerialStreamBuf(ISerialDriver &&driver)
        : m_Driver{driver},
          m_InBuf(4096),
          m_OutBuf(4096)
{
    // empty get area
    setg(reinterpret_cast<char*>(m_InBuf.data()),
         reinterpret_cast<char*>(m_InBuf.data()),
         reinterpret_cast<char*>(m_InBuf.data()));

    // empty put area
    setp(reinterpret_cast<char*>(m_OutBuf.data()),
         reinterpret_cast<char*>(m_OutBuf.data() + m_OutBuf.size()));
}

wincom::SerialStreamBuf::~SerialStreamBuf()
{
    SerialStreamBuf::sync();
}

wincom::SerialStreamBuf::int_type wincom::SerialStreamBuf::underflow()
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
int wincom::SerialStreamBuf::sync()
{
    return flushOut_() ? 0 : -1;
}

std::streambuf::int_type wincom::SerialStreamBuf::overflow(int_type ch)
{
    if (!traits_type::eq_int_type(ch, traits_type::eof()))
    {
        *pptr() = traits_type::to_char_type(ch);
        pbump(1);
    }
    return flushOut_() ? traits_type::not_eof(ch) : traits_type::eof();
}

std::streamsize wincom::SerialStreamBuf::xsputn(const char* s, std::streamsize n)
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

std::streamsize wincom::SerialStreamBuf::xsgetn(char* s, std::streamsize n)
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

std::streambuf::pos_type wincom::SerialStreamBuf::seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode)
{
    return {static_cast<off_type>(-1)};
}

std::streambuf::pos_type wincom::SerialStreamBuf::seekpos(pos_type, std::ios_base::openmode)
{
    return {static_cast<off_type>(-1)};
}


bool wincom::SerialStreamBuf::flushOut_()
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
            reinterpret_cast<const uint8_t *>(pbase()) + written,
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
                     reinterpret_cast<const uint8_t *>(pbase()) + written,
                     remaining);
    }

    setp(pbase(), epptr());
    pbump(static_cast<int>(remaining));
    return written > 0 || remaining == 0;
}

[[nodiscard]] std::chrono::milliseconds wincom::SerialStreamBuf::timeoutForRead_() const
{
    switch (m_Driver.getTimeoutPolicy().mode)
    {
        case ISerialDriver::TimeoutMode::blocking: return std::chrono::milliseconds{-1}; // sentinel: block forever
        case ISerialDriver::TimeoutMode::finite: return m_Driver.getTimeoutPolicy().readTimeout;
        case ISerialDriver::TimeoutMode::nonBlocking: return std::chrono::milliseconds{0};
    }
    return m_Driver.getTimeoutPolicy().readTimeout;
}

[[nodiscard]] std::chrono::milliseconds wincom::SerialStreamBuf::timeoutForWrite_() const
{
    switch (m_Driver.getTimeoutPolicy().mode)
    {
        case ISerialDriver::TimeoutMode::blocking: return std::chrono::milliseconds{-1};
        case ISerialDriver::TimeoutMode::finite: return m_Driver.getTimeoutPolicy().writeTimeout;
        case ISerialDriver::TimeoutMode::nonBlocking: return std::chrono::milliseconds{0};
    }
    return m_Driver.getTimeoutPolicy().writeTimeout;
}
