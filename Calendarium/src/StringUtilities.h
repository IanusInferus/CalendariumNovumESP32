#pragma once

#include <string>

std::u16string utf8ToUtf16(const std::string & u8s);
std::string utf16ToUtf8(const std::u16string & u16s);
std::u32string utf8ToUtf32(const std::string & u8s);
std::string utf32ToUtf8(const std::u32string & u32s);
std::u16string utf32ToUtf16(const std::u32string & u32s);
std::u32string utf16ToUtf32(const std::u16string & u16s);
