#pragma once
#include <iosfwd>
#include <streambuf>
#include <vector>

class vector_streambuf final : public std::streambuf
{
public:
    explicit vector_streambuf(std::vector<std::byte>& vec) : vec_(vec)
    {
    }

protected:
    int_type overflow(int_type ch) override
    {
        if (ch != EOF)
        {
            vec_.push_back(static_cast<std::byte>(ch));
        }
        return ch;
    }

    std::streamsize xsputn(const char* s, std::streamsize n) override
    {
        const auto start = reinterpret_cast<const std::byte*>(s);
        vec_.insert(vec_.end(), start, start + n);
        return n;
    }

private:
    std::vector<std::byte>& vec_;
};
