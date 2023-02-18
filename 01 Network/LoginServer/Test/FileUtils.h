#pragma once
#include <stdio.h>
#include <memory>

void LoadFile(const char* fileName, char** dst) {
    FILE* fp;
    fopen_s(&fp, fileName, "rt");
    if (!fp) {
        return;
    }

    // Get fileSize
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    *dst = (char*)malloc(fileSize + 1);
    fread(*dst, 1, fileSize, fp);
    (*dst)[fileSize] = '\0';
    fclose(fp);
}

char* LoadFile(const char* fileName) {
    FILE* fp;
    fopen_s(&fp, fileName, "rt");
    if (!fp) {
        return nullptr;
    }

    // Get fileSize
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    char* dst = (char*)malloc(fileSize + 1);
    fread(dst, 1, fileSize, fp);
    dst[fileSize] = '\0';
    fclose(fp);

    return dst;
}

char* FindSubstr(const char* str, const char* sub, const char* exclude = nullptr) {
    const int strLen = std::strlen(str);
    const int subLen = std::strlen(sub);
    const int exLen = (exclude == nullptr) ? 0 : std::strlen(exclude);

    for (int offset = 0; offset < strLen; offset++) {
        // 주석 발견. 개행문자가 나올때까지 스킵
        if (str[offset] == '/' && offset < strLen - 1 && str[offset + 1] == '/') {
            while (offset < strLen && str[offset] != '\n') {
                offset++;
            }
        }
        // 비교할 필요 없는 문자들 건너뛰기
        else if (str[offset] == ' ' || str[offset] == '\n' || str[offset] == '\t') {
            continue;
        }
        // str 남은 문자열에 sub가 속할 수 없음. 함수 반환
        else if (strLen < offset + subLen) {
            return nullptr;
        }
        // str 남은 문자열에 exclude가 속할 수 있음. exclude 발견 시 함수 종료
        else if (exclude != nullptr && offset + exLen <= strLen) {
            if (std::strncmp(str + offset, exclude, exLen) == 0) {
                return nullptr;
            }
        }
        else {
            // 문자비교
            if (std::strncmp(str + offset, sub, subLen) == 0) {
                return (char*)&str[offset + subLen];
            }
        }
    }
    return nullptr;
}

char* FindSubchar(const char* str, const char sub) {
    const int strLen = std::strlen(str);

    for (int offset = 0; offset < strLen; offset++) {
        // 주석 발견. 개행문자가 나올때까지 스킵
        if (str[offset] == '/' && offset < strLen - 1 && str[offset + 1] == '/') {
            while (offset < strLen && str[offset] != '\n') {
                offset++;
            }
        }
        else if (str[offset] == sub) {
			return (char*)&str[offset + 1];
        }
        // 비교할 필요 없는 문자들 건너뛰기
		else if (str[offset] == ' ' || str[offset] == '\n' || str[offset] == '\t') {
			continue;
		}
        else {
			return nullptr;
        }
    }
    return nullptr;
}

bool Findstr(const char* str, char* dst, const char* exclude = nullptr) {
    const int strLen = std::strlen(str);
    const int exLen = (exclude == nullptr) ? 0 : std::strlen(exclude);

    int dstOffest = 0;
    for (int offset = 0; offset < strLen; offset++) {
        // 주석 발견. 개행문자가 나올때까지 스킵
        if (str[offset] == '/' && offset < strLen - 1 && str[offset + 1] == '/') {
            while (offset < strLen && str[offset] != '\n') {
                offset++;
            }
        }
		// 보통 건너뜀. dst의 시작 혹은 끝일 수 있음.
		else if (str[offset] == ' ' || str[offset] == '\n' || str[offset] == '\t') {
			if (0 == dstOffest)
				continue;
            return true;
        }
        // str 남은 문자열에 exclude가 속할 수 있음. exclude 발견 시 함수 종료
        else if (exclude != nullptr && offset + exLen <= strLen) {
            if (std::strncmp(str + offset, exclude, exLen) == 0) {
                return false;
            }
        }
        else {
            dst[dstOffest++] = str[offset];
        }
    }
    return false;
}