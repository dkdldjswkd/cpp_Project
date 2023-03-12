#pragma once

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

#define LAN_HEADER_SIZE  ((WORD)sizeof(LAN_HEADER))
#define NET_HEADER_SIZE  ((WORD)sizeof(NET_HEADER))