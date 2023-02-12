#include "stdafx.h"
#include "Error_Check.h"

#undef Error_Check

// 에러 상황을 넣어야함
int Error_Check(const char* file_name, int line, bool is_error)
{
	if (!is_error)
		return true;

	auto error_num = WSAGetLastError();

	if (error_num == WSAECONNRESET)
		return WSAECONNRESET;

	// 크래시
	printf("\n///////////////////////////////////////////////////////\n");
	printf("//  WSAGetLastError() : %d \n", WSAGetLastError());
	printf("//  __FILE__ : %s \n", file_name);
	printf("//  __LINE__ : %d \n", line);
	printf("///////////////////////////////////////////////////////\n");
	
	CRASH();
	return false;
}