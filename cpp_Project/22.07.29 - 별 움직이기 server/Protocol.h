#pragma once

#define PACKET_SIZE 16

#define PACKET_GET_ID 0
#define PACKET_CREATE_CHARACTER 1
#define PACKET_REMOVE_CHARACTER 2
#define PACKET_MOVE 3

struct packet_get_id {
	int type;
	int id;
	long long zero;
};

struct packet_create_character {
	int type;
	int id;
	unsigned int x, y;
};

struct packet_remove_character {
	int type;
	int id;
	long long zero;
};

struct packet_move {
	int type;
	int id;
	unsigned int x, y;
};
