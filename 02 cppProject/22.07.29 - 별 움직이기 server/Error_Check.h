#pragma once

// 크래쉬 내는 코드
#define CRASH() do{ \
	char* crash = nullptr; \
	(*crash)++; \
}while(0)

// 터트리는 코드

int Error_Check(const char* file_name, int line, bool is_error);
#define Error_Check(is_error) Error_Check(__FILE__, __LINE__, is_error)