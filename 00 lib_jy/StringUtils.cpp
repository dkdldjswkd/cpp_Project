#include "StringUtils.h"
#include <Windows.h>

void UTF8ToUTF16(const char* src, std::wstring& dst) {
    int size = MultiByteToWideChar(CP_UTF8, 0, src, -1, nullptr, 0);
    dst.resize(size - 1);
    MultiByteToWideChar(CP_UTF8, 0, src, -1, &dst[0], size);
}

void UTF8ToUTF16(const char* src, const wchar_t* dst) {
    MultiByteToWideChar(CP_UTF8, 0, src, -1, (LPWSTR)dst, strlen(src) + 1);
}  