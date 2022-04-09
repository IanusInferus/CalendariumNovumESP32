#include "StringUtilities.h"

#include <utf8.h>

std::u16string utf8ToUtf16(const std::string & u8s)
{
    std::u16string u16s;
    u16s.reserve(u8s.size());
    utf8::utf8to16(u8s.begin(), u8s.end(), std::back_inserter(u16s));
    return u16s;
}

std::string utf16ToUtf8(const std::u16string & u16s)
{
    std::string u8s;
    u8s.reserve(u16s.size() * 2);
    utf8::utf16to8(u16s.begin(), u16s.end(), std::back_inserter(u8s));
    return u8s;
}

std::u32string utf8ToUtf32(const std::string & u8s)
{
    std::u32string u32s;
    u32s.reserve(u8s.size());
    utf8::utf8to32(u8s.begin(), u8s.end(), std::back_inserter(u32s));
    return u32s;
}

std::string utf32ToUtf8(const std::u32string & u32s)
{
    std::string u8s;
    u8s.reserve(u32s.size() * 4);
    utf8::utf32to8(u32s.begin(), u32s.end(), std::back_inserter(u8s));
    return u8s;
}

std::u16string utf32ToUtf16(const std::u32string & u32s)
{
    return utf8ToUtf16(utf32ToUtf8(u32s));
}

std::u32string utf16ToUtf32(const std::u16string & u16s)
{
    return utf8ToUtf32(utf16ToUtf8(u16s));
}
