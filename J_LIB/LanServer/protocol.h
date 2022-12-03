#pragma once

struct LAN_HEADER {
	unsigned short len;
};

constexpr unsigned LAN_HEADER_SIZE = sizeof(LAN_HEADER);
constexpr unsigned HEADER_SIZE = sizeof(LAN_HEADER);