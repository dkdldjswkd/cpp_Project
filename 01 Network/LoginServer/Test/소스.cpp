#include <iostream>
#include <Windows.h>
#include <string>
#include "FileUtils.h"
using namespace std;

struct Parser {
public:
	Parser() {};
	~Parser() {};

private:
	char* fileBegin = nullptr;
	char* fileCur = nullptr;
	char* fileEnd = nullptr;

private:
	bool FindChar();
	bool FindSpace();
	bool FindWord(const char* word);
	bool NextChar();
    bool GetValueInt(int* value);
    bool GetValueStr(char* value);

public:
	bool LoadFile(const char* file);
	bool GetValue(const char* section, const char* key, int* value);
	bool GetValue(const char* section, const char* key, char* value);
};

// ����� �ּ��� ������ ���ڰ� ���ö����� fileCur�� �̵���Ŵ
bool Parser::FindChar(){ 
	for (; fileCur < fileEnd; fileCur++) {
        // �ּ� �߰�. ���๮�ڰ� ���ö����� �پ�ѱ�
        if (*fileCur == '/'  && *(fileCur+1) == '/') {
            for (; fileCur < fileEnd - 1; fileCur++) {
                if (*fileCur == '\n')
                    break;
            }
        }
        // ���� ���� �پ�ѱ�
        else if (*fileCur == ' ' || *fileCur == '\n' || *fileCur == '\t') {
            continue;
        }
        else {
            return true;
        }
    }
    return false;
}

bool Parser::FindSpace(){
    for (; fileCur < fileEnd; fileCur++) {
        // �ּ� �߰�. ���๮�ڰ� ���ö����� �پ�ѱ�
        if (*fileCur == '/' && *(fileCur + 1) == '/') {
            for (; fileCur < fileEnd - 1; fileCur++) {
                if (*fileCur == '\n')
                    break;
            }
        }
        // ���� �߰�
        else if (*fileCur == ' ' || *fileCur == '\n' || *fileCur == '\t') {
            return true;
        }
    }
    return false;
}

bool Parser::FindWord(const char* word){
    int wordLen = strlen(word);

    if (!FindChar()) return false;
    for (;;) {
        if (0 == strncmp(fileCur, word, wordLen)) {
            // �ܾ� ã��
            if (*(fileCur + wordLen)== ' ' || *(fileCur + wordLen) == '\t' || *(fileCur + wordLen) == '\n') {
                return true;
            }
        }
        if (!NextChar()) return false;
    }
    return false;
}

bool Parser::NextChar(){
    if (false == FindSpace())
        return false;
    return FindChar();
}

bool Parser::GetValueInt(int* value) {
    char buf[100];
    int offset = 0;

    // value ã��
	for (;;) {
        // ���� �ƴ�.
		if (*fileCur < '0' || '9' < *fileCur) {
			return false;
		}
		for (;;) {
            if (*fileCur < '0' || '9' < *fileCur) {
				*value = atoi(buf);
                return true;
            }
            buf[offset++] = *fileCur;
            fileCur++;
        }
    }
    return true;
}

bool Parser::GetValueStr(char* value) {
    int offset = 0;

    // value ã��
    for (;;) {
        // ���ڿ� �ƴ�
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

bool Parser::LoadFile(const char* file){
    if (fileBegin != nullptr)
        free(fileBegin);

    FILE* fp;
    fopen_s(&fp, file, "rt");
    if (!fp) {
        return false;
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
    return true;
}

bool Parser::GetValue(const char* section, const char* key, int* value){
    fileCur = fileBegin;
    int sectionLen = strlen(section);
    int keyLen = strlen(key);

    // Section ã��
    if(false == FindWord(section)) return false;

    // { ã��
    NextChar();
	if (*fileCur != '{') {
		return false;
	}

    // Section ã��
    if (false == FindWord(key)) return false;

    // = ã��
    NextChar();
    if (*fileCur != '=') {
        return false;
    }

    // value ã��
    NextChar();
    GetValueInt(value);
}

bool Parser::GetValue(const char* section, const char* key, char* value){
    fileCur = fileBegin;
    int sectionLen = strlen(section);
    int keyLen = strlen(key);

    // Section ã��
    if (false == FindWord(section)) return false;

    // { ã��
    NextChar();
    if (*fileCur != '{') {
        return false;
    }

    // Section ã��
    if (false == FindWord(key)) return false;

    // = ã��
    NextChar();
    if (*fileCur != '=') {
        return false;
    }

    // value ã��
    NextChar();
    GetValueStr(value);
}

int main() {
    Parser parser;
    parser.LoadFile("test.txt");
    char ip[100];
    int port;

    parser.GetValue("CHAT_SERVER", "IP", ip);
    parser.GetValue("CHAT_SERVER", "PORT", &port);
    printf("ip : %s, port %d\n", ip, port);

    parser.GetValue("CHAT_SERVER", "IP", ip);
    parser.GetValue("CHAT_SERVER", "PORT", &port);
    printf("ip : %s, port %d\n", ip, port);

    parser.GetValue("CHAT_SERVER", "IP", ip);
    parser.GetValue("CHAT_SERVER", "PORT", &port);
    printf("ip : %s, port %d\n", ip, port);
}