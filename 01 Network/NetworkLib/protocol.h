#pragma once

#pragma pack(push, 1)
struct LanHeader {
	WORD len;
};

struct NetHeader {
	BYTE code;
	WORD len;
	BYTE randKey;
	BYTE checkSum;
};
#pragma pack(pop)

#define LAN_HEADER_SIZE  ((WORD)sizeof(LanHeader))
#define NET_HEADER_SIZE  ((WORD)sizeof(NetHeader))