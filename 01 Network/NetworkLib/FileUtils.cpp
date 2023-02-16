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
		// �ּ� �߰�. ���๮�ڰ� ���ö����� ��ŵ
        if (str[offset] == '/' && offset < strLen - 1 && str[offset + 1] == '/') {
            while (offset < strLen && str[offset] != '\n') {
                offset++;
            }
		}
		// ���� �ʿ� ���� ���ڵ� �ǳʶٱ�
		else if (str[offset] == ' ' || str[offset] == '\n' || str[offset] == '\t') {
			continue;
		}
		// str ���� ���ڿ��� sub�� ���� �� ����. �Լ� ��ȯ
		else if (strLen < offset + subLen) {
			return nullptr;
		}
		// str ���� ���ڿ��� exclude�� ���� �� ����. exclude �߰� �� �Լ� ����
		else if (offset + exLen <= strLen) {
			if (std::strncmp(str + offset, exclude, exLen) == 0) {
				return nullptr;
			}
		}
		else {
			// ���ں�
			if (std::strncmp(str + offset, sub, subLen) == 0) {
				return (char*)&str[offset + subLen];
			}
		}
	}
	return nullptr;
}