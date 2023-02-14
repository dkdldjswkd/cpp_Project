#pragma once
#include <string>
void JStrcpy(const char* src, std::string& dst);
void JStrcpy(const char* src, char* dst, int size);
void UTF8ToUTF16(const char* src, std::wstring& dst);
void UTF8ToUTF16(const char* src, const wchar_t* dst);