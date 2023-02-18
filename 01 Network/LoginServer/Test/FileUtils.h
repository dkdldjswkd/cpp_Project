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
        else if (exclude != nullptr && offset + exLen <= strLen) {
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

char* FindSubchar(const char* str, const char sub) {
    const int strLen = std::strlen(str);

    for (int offset = 0; offset < strLen; offset++) {
        // �ּ� �߰�. ���๮�ڰ� ���ö����� ��ŵ
        if (str[offset] == '/' && offset < strLen - 1 && str[offset + 1] == '/') {
            while (offset < strLen && str[offset] != '\n') {
                offset++;
            }
        }
        else if (str[offset] == sub) {
			return (char*)&str[offset + 1];
        }
        // ���� �ʿ� ���� ���ڵ� �ǳʶٱ�
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
        // �ּ� �߰�. ���๮�ڰ� ���ö����� ��ŵ
        if (str[offset] == '/' && offset < strLen - 1 && str[offset + 1] == '/') {
            while (offset < strLen && str[offset] != '\n') {
                offset++;
            }
        }
		// ���� �ǳʶ�. dst�� ���� Ȥ�� ���� �� ����.
		else if (str[offset] == ' ' || str[offset] == '\n' || str[offset] == '\t') {
			if (0 == dstOffest)
				continue;
            return true;
        }
        // str ���� ���ڿ��� exclude�� ���� �� ����. exclude �߰� �� �Լ� ����
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