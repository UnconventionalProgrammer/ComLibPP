#ifndef WINCOMLIBPP_HPP
#define WINCOMLIBPP_HPP


#include <cstdint>
#include <cstring>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <streambuf>
#include <string>
#include <vector>

#include "ISerialDriver.hpp"
#include "export.hpp"
#include "Win32SerialDriver.hpp"

namespace wincom
{

class SerialStreamBuf final : public std::streambuf
{
public:
    explicit SerialStreamBuf(ISerialDriver &&driver);

    ~SerialStreamBuf() override;
    SerialStreamBuf(const SerialStreamBuf&) = delete;
    SerialStreamBuf& operator=(const SerialStreamBuf&) = delete;

protected:
    int_type underflow() override; // refill get area
    int sync() override; // flush put area
    int_type overflow(int_type ch) override; // write one char (or flush)
    std::streamsize xsputn(const char* s, std::streamsize n) override; // write block
    std::streamsize xsgetn(char* s, std::streamsize n) override; // read block
    pos_type seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode) override; // serial ports are not seekable
    pos_type seekpos(pos_type, std::ios_base::openmode) override; // serial ports are not seekable

private:
    bool flushOut_();
    [[nodiscard]] std::chrono::milliseconds timeoutForRead_() const;
    [[nodiscard]] std::chrono::milliseconds timeoutForWrite_() const;

private:
    ISerialDriver&         m_Driver;
    std::vector<uint8_t>   m_InBuf;
    std::vector<uint8_t>   m_OutBuf;
};


// ------------------------------
// iostream facade
// ------------------------------

template <typename Driver>
class SerialStream final : public std::iostream
{
public:
    template <typename ...Args>
    explicit SerialStream(Args ...args)
        : std::iostream(nullptr),
          m_Buf(Driver{std::forward<Args>(args)...})
    {
        std::iostream::rdbuf(&m_Buf);
        unsetf(std::ios::skipws); // binary-friendly
    }

    SerialStreamBuf* rdbuf()
    {
        return &m_Buf;
    }

private:
    SerialStreamBuf m_Buf;
};

} // namespace wincom


#endif