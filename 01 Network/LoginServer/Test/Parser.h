#pragma once

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