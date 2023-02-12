#pragma once

//------------------------------
// Network
//------------------------------
constexpr unsigned SYNC_RANGE = 50;
constexpr unsigned NETWORK_PORT = 10750;
constexpr unsigned dfNETWORK_PACKET_RECV_TIMEOUT = 30000;

//------------------------------
// Sector
//------------------------------
constexpr unsigned RANGE_MOVE_TOP = 0;
constexpr unsigned RANGE_MOVE_LEFT = 0;
constexpr unsigned RANGE_MOVE_RIGHT = 6400;
constexpr unsigned RANGE_MOVE_BOTTOM = 6400;

constexpr unsigned SECTOR_SIZE = 200;
constexpr unsigned SECTOR_MAX_X = RANGE_MOVE_RIGHT / SECTOR_SIZE; //32
constexpr unsigned SECTOR_MAX_Y = RANGE_MOVE_BOTTOM / SECTOR_SIZE;

//------------------------------
// Content
//------------------------------
constexpr unsigned ATTACK1_RANGE_X = 80;
constexpr unsigned ATTACK2_RANGE_X = 90;
constexpr unsigned ATTACK3_RANGE_X = 100;
constexpr unsigned ATTACK1_RANGE_Y = 10;
constexpr unsigned ATTACK2_RANGE_Y = 10;
constexpr unsigned ATTACK3_RANGE_Y = 20;

constexpr unsigned ATTACK1_DAMAGE = 1;
constexpr unsigned ATTACK2_DAMAGE = 2;
constexpr unsigned ATTACK3_DAMAGE = 3;

constexpr unsigned SPEED_PLAYER_X = 6;	// 3   50fps
constexpr unsigned SPEED_PLAYER_Y = 4;	// 2   50fps

//------------------------------
// Debug
//------------------------------
constexpr unsigned char DEBUF_LEVEL = 1; // 1. 모니터링, 2. Disconnect, PACKET 3. 캐릭터 이동 로그

extern int session_count;
extern int character_count;
extern int syncMsg_count;
extern int moveStart_sync;
extern int moveStop_sync ;
extern int attack_sync;
extern int sessionPool_count;
extern int characterPool_count;
extern int fin_count;
extern int disconnect_count;

//------------------------------
// Monitoring
//------------------------------
extern DWORD monitor_timer;
extern DWORD logic_frame;

#define FRAME_PER_SEC	25							// 25fps
#define FRAME_MS		40 //(DWORD)(1000/FRAME_PER_SEC) // 40ms
//#define FRAME_MS		(DWORD)(1000/FRAME_PER_SEC) // 40ms