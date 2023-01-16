#pragma once

#define PROTOCOL_CODE 0x89
#define CONST_KEY 0xa9

struct LAN_HEADER {
	WORD len;
};

struct NET_HEADER {
	BYTE code;
	WORD len;
	BYTE randKey;
	BYTE checkSum;
};

constexpr unsigned LAN_HEADER_SIZE = sizeof(LAN_HEADER);
constexpr unsigned NET_HEADER_SIZE = sizeof(NET_HEADER);