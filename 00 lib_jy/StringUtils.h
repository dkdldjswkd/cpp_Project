#pragma once
#include <string>
void UTF8ToUTF16(const char* src, std::wstring& dst);
void UTF8ToUTF16(const char* src, const wchar_t* dst);