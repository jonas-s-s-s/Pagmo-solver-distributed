#pragma once
#include <iosfwd>
#include <streambuf>
#include <vector>

class vector_istreambuf final : public std::streambuf
{
public:
    explicit vector_istreambuf(const std::vector<std::byte>& vec)
    {
        char* begin = const_cast<char*>(reinterpret_cast<const char*>(vec.data()));
        setg(begin, begin, begin + vec.size());
    }
};
