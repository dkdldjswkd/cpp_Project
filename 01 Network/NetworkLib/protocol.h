#pragma once
#include <Windows.h>
#define PROTOCOL_CODE	0x77
#define CONST_KEY		0x32

#pragma pack(push, 1)
struct LAN_HEADER {
	WORD len;
};

struct NET_HEADER {
	BYTE code;
	WORD len;
	BYTE randKey;
	BYTE checkSum;
};
#pragma pack(pop)

constexpr unsigned LAN_HEADER_SIZE = sizeof(LAN_HEADER);
constexpr unsigned NET_HEADER_SIZE = sizeof(NET_HEADER);