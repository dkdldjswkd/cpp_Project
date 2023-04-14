#include "Parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Parser::Parser(){
}

Parser::~Parser(){
    if (fileBegin != nullptr) {
        free(fileBegin);
    }
}

// 공백과 주석을 제외한 문자가 나올때까지 fileCur을 이동시킴
bool Parser::FindChar() {
    for (; fileCur < fileEnd; fileCur++) {
        // 주석 발견. 개행문자가 나올때까지 뛰어넘기
        if (*fileCur == '/' && *(fileCur + 1) == '/') {
            for (; fileCur < fileEnd - 1; fileCur++) {
                if (*fileCur == '\n')
                    break;
            }
        }
        // 공백 문자 뛰어넘기
        else if (*fileCur == ' ' || *fileCur == '\n' || *fileCur == '\t') {
            continue;
        }
        else {
            return true;
        }
    }
    return false;
}

bool Parser::FindSpace() {
    for (; fileCur < fileEnd; fileCur++) {
        // 주석 발견. 개행문자가 나올때까지 뛰어넘기
        if (*fileCur == '/' && *(fileCur + 1) == '/') {
            for (; fileCur < fileEnd - 1; fileCur++) {
                if (*fileCur == '\n')
                    break;
            }
        }
        // 공백 발견
        else if (*fileCur == ' ' || *fileCur == '\n' || *fileCur == '\t') {
            return true;
        }
    }
    return false;
}

bool Parser::FindWord(const char* word) {
    int wordLen = strlen(word);

    if (!FindChar()) return false;
    for (;;) {
        if (0 == strncmp(fileCur, word, wordLen)) {
            // 단어 찾음
            if (*(fileCur + wordLen) == ' ' || *(fileCur + wordLen) == '\t' || *(fileCur + wordLen) == '\n') {
                return true;
            }
        }
        if (!NextChar()) return false;
    }
    return false;
}

bool Parser::NextChar() {
    if (false == FindSpace())
        return false;
    return FindChar();
}

bool Parser::GetValueInt(int* value) {
    *value = 0;

    for (;;) {
        // 숫자 아님.
        if (*fileCur < '0' || '9' < *fileCur) {
            return false;
        }
        for (;;) {
            if (*fileCur < '0' || '9' < *fileCur) {
                return true;
            }
            *value *= 10;
            *value += *fileCur - '0';
            fileCur++;
        }
    }
    return true;
}

bool Parser::GetValueStr(char* value) {
    int offset = 0;

    // value 찾기
    for (;;) {
        // 문자열 아님
        if (*fileCur != '"') {
            return false;
        }
        fileCur++;
        for (;;) {
            if (*fileCur == '"') {
                value[offset] = 0;
                return true;
            }
            value[offset++] = *fileCur;
            fileCur++;
        }
    }
    return true;
}

int Parser::LoadFile(const char* file) {
    if (fileBegin != nullptr) {
        free(fileBegin);
    }

    FILE* fp;
    fopen_s(&fp, file, "rt");
    if (!fp) {
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    fileBegin = (char*)malloc(fileSize + 1);
    fileCur = fileBegin;
    fileEnd = fileBegin + fileSize + 1;

    fread(fileBegin, 1, fileSize, fp);
    fileBegin[fileSize] = '\0';
    fclose(fp);
    return fileSize;
}

bool Parser::GetValue(const char* section, const char* key, int* value) {
    fileCur = fileBegin;
    int sectionLen = strlen(section);
    int keyLen = strlen(key);

    // Section 찾기
    if (false == FindWord(section)) return false;

    // { 찾기
    NextChar();
    if (*fileCur != '{') {
        return false;
    }

    // Section 찾기
    if (false == FindWord(key)) return false;

    // = 찾기
    NextChar();
    if (*fileCur != '=') {
        return false;
    }

    // value 찾기
    NextChar();
    return GetValueInt(value);
}

bool Parser::GetValue(const char* section, const char* key, char* value) {
    fileCur = fileBegin;
    int sectionLen = strlen(section);
    int keyLen = strlen(key);

    // Section 찾기
    if (false == FindWord(section)) return false;

    // { 찾기
    NextChar();
    if (*fileCur != '{') {
        return false;
    }

    // Section 찾기
    if (false == FindWord(key)) return false;

    // = 찾기
    NextChar();
    if (*fileCur != '=') {
        return false;
    }

    // value 찾기
    NextChar();
    return GetValueStr(value);
}