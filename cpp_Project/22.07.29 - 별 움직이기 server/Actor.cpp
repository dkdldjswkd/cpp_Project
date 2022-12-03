#include "stdafx.h"
#include "Actor.h"
#include "Error_Check.h"


Player::Player() : send_buf(128), recv_buf(128) {}

void Player::Invalidate() {
	Error_Check(SOCKET_ERROR == closesocket(player_socket));

	player_socket = INVALID_SOCKET;
	ip = 0;
	port = 0;

	id = INVALID_ID;
	x = 10;
	y = 10;

	flag_disconnect = false;
}
