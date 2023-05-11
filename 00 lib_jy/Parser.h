#pragma once

struct Parser {
public:
	Parser();
	~Parser();

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
	int LoadFile(const char* file); // return file len
	bool GetValue(const char* section, const char* key, int* value);
	bool GetValue(const char* section, const char* key, char* value);
};

/*
Parser 사용 방법

////////////////////////////////
// 파싱 파일 설명
////////////////////////////////

// 파일명으로 가정
Config.ini

// 파일 내용으로 가정
Monster					// GetValue 함수의 section에 해당합니다.
{
						// * value의 자료형에 해당하는 GetValue 함수를 호출할것
	NAME	= "Golem"	// NAME은 key, "Golem"이 value에 해당합니다.
	ATTACK	= 30000		// ATTACK은 key, 30000이 value에 해당합니다.
}

////////////////////////////////
// Paser class 사용 예
////////////////////////////////

char name[20];
short attack;

Parser parser;
parser.LoadFile("Config.ini");
GetValue("Monster", "NAME", name);
GetValue("Monster", "ATTACK", &attack);

*/