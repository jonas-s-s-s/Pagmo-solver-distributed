#pragma once

#include <filesystem>
#include <string>
#include <cctype>

constexpr std::string_view WIN_RESERVED_KEYWORDS[] = {
    "CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2",
    "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
};

constexpr std::string_view WIN_RESERVED_SYMBOLS = "<>:\"/\\|?*";

inline std::filesystem::path win_normalize_filename(const std::filesystem::path& p)
{
    std::string s = p.filename().string();

    for (char& c : s)
        if (WIN_RESERVED_SYMBOLS.find(c) != std::string_view::npos)
            c = '_';

    while (!s.empty() && (s.back() == ' ' || s.back() == '.'))
        s.pop_back();

    if (s.empty())
        s = "file";

    std::string upper = s;
    for (char& c : upper)
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    for (const auto r : WIN_RESERVED_KEYWORDS)
        if (upper == r)
        {
            s.insert(s.begin(), '_');
            break;
        }

    return p.parent_path() / s;
}
