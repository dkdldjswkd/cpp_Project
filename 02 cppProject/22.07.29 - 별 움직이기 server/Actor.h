#pragma once

#include "RingBuffer.h"

#define INVALID_ID -1

struct Player {
public:
	Player();
	void Invalidate();

public:
	SOCKET			player_socket = INVALID_SOCKET;
	RingBuffer		send_buf;
	RingBuffer		recv_buf;

	int				ip = 0;
	short			port = 0;

	char	id = INVALID_ID;
	unsigned char	x = 10;
	unsigned char	y = 10;

	bool flag_disconnect = false;
};
