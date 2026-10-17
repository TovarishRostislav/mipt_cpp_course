#pragma once

#include <string>

namespace nano_edr{
constexpr char MyToLower(char c) noexcept{
    if(c >= 'A' && c <= 'Z') return static_cast<char>(c + ('a' - 'A'));
    return c;
}

bool EqualsIgnoreCase(const std::string& a, const std::string& b);
bool ContainsIgnoreCase(const std::string& haystack, const std::string& needle);
bool EndsWithIgnoreCase(const std::string& haystack, const std::string& suff);
}