#include "StringUtils.h"
#include <Windows.h>

void JStrcpy(const char* src, std::string& dst) {
    dst.resize(strlen(src));
    memcpy(&dst[0], src, dst.size());
}

void JStrcpy(const char* src, char* dst, int size) {
    int src_len = strlen(src);
    int copy_len = (src_len < size - 1) ? src_len : size - 1;
    memcpy(dst, src, copy_len);
    dst[copy_len] = 0;
}

void UTF8ToUTF16(const char* src, std::wstring& dst) {
    int size = MultiByteToWideChar(CP_UTF8, 0, src, -1, nullptr, 0);
    dst.resize(size - 1);
    MultiByteToWideChar(CP_UTF8, 0, src, -1, &dst[0], size);
}

void UTF8ToUTF16(const char* src, const wchar_t* dst) {
    MultiByteToWideChar(CP_UTF8, 0, src, -1, (LPWSTR)dst, strlen(src) + 1);
}