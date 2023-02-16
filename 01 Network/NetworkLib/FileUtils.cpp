#include <iostream>
#include "FileUtils.h"

void LoadFile(const char* fileName, char** dst) {
	FILE* fp = NULL;
	fopen_s(&fp, fileName, "rt");

	// Get fileSize
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);
	rewind(fp);

	*dst = (char*)malloc(fileSize + 1);
	fread(*dst, 1, fileSize, fp);
	(*dst)[fileSize] = '\0';
	fclose(fp);
}

//void LoadFileW(const wchar_t* fileName, wchar_t** dst) {
//	FILE* fp = NULL;
//	_wfopen_s(&fp, fileName, L"rt");
//
//	fseek(fp, 0, SEEK_END);
//	int fileSize = ftell(fp);
//	rewind(fp);
//
//	// Allocate memory for the buffer
//	*dst = (wchar_t*)malloc(sizeof(wchar_t) * (fileSize + 1));
//	if (*dst == NULL) {
//		fclose(fp);
//	}
//
//	// Read the file into the buffer
//	fread(*dst, sizeof(wchar_t), fileSize, fp);
//	(*dst)[fileSize / sizeof(wchar_t)] = L'\0';
//
//	fclose(fp);
//}

char* FindSubstr(const char* str, const char* sub, const char* exclude) {
    const int strLen = std::strlen(str);
    const int subLen = std::strlen(sub);
    const int exLen = std::strlen(exclude);

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
		else if (offset + exLen <= strLen) {
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