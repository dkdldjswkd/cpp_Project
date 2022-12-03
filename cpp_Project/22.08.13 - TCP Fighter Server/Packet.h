#pragma once
#include "PacketDefine.h"

// 이동 오류체크 범위
#define ERROR_RANGE		50

// 화면 이동영역, 프레임당 이동 단위 -  X : 3, Y : 2
#define RANGE_MOVE_TOP		50
#define RANGE_MOVE_BOTTOM	470
#define RANGE_MOVE_LEFT		10
#define RANGE_MOVE_RIGHT	630

// 공격범위
#define ATTACK1_RANGE_X		80
#define ATTACK1_RANGE_Y		10

#define ATTACK2_RANGE_X		90
#define ATTACK2_RANGE_Y		10

#define ATTACK3_RANGE_X		100
#define ATTACK3_RANGE_Y		20

// 데미지
#define ATTACK1_DAMAGE 5
#define ATTACK2_DAMAGE 10
#define ATTACK3_DAMAGE 15


enum EDir {
	PACKET_MOVE_DIR_LL,
	PACKET_MOVE_DIR_LU,
	PACKET_MOVE_DIR_UU,
	PACKET_MOVE_DIR_RU,
	PACKET_MOVE_DIR_RR,
	PACKET_MOVE_DIR_RD,
	PACKET_MOVE_DIR_DD,
	PACKET_MOVE_DIR_LD,
};

#pragma pack(push, 1)

struct PACKET_HEADER {
	unsigned char code = 0x89;	// 패킷코드 0x89 고정
	unsigned char size = 0;			// 패킷 사이즈
	unsigned char type = ~0;			// 패킷 타입
};

// #define PACKET_SC_CREATE_MY_CHARACTER			0
//---------------------------------------------------------------
// 클라이언트 자신의 캐릭터 할당		Server -> Client
//
// 서버에 접속시 최초로 받게되는 패킷으로 자신이 할당받은 ID 와
// 자신의 최초 위치, HP 를 받게 된다. (처음에 한번 받게 됨)
// 
// 이 패킷을 받으면 자신의 ID,X,Y,HP 를 저장하고 캐릭터를 생성시켜야 한다.
//---------------------------------------------------------------
struct	packet_sc_create_my_character {
	unsigned int id;
	unsigned char dir;
	unsigned short x;
	unsigned short y;
	char hp;
};

//#define	PACKET_SC_CREATE_OTHER_CHARACTER		1
//---------------------------------------------------------------
// 다른 클라이언트의 캐릭터 생성 패킷		Server -> Client
//
// 처음 서버에 접속시 이미 접속되어 있던 캐릭터들의 정보
// 또는 게임중에 접속된 클라이언트들의 생성용 정보.
//---------------------------------------------------------------
struct	packet_sc_create_other_character {
	unsigned int id;
	unsigned char dir;
	unsigned short x;
	unsigned short y;
	char hp;
};

//#define	PACKET_SC_DELETE_CHARACTER			2
//---------------------------------------------------------------
// 캐릭터 삭제 패킷						Server -> Client
//
// 캐릭터의 접속해제 또는 캐릭터가 죽었을때 전송됨.
//---------------------------------------------------------------
struct	packet_sc_delete_character {
	unsigned int id;
};

//#define	PACKET_CS_MOVE_START					10
//---------------------------------------------------------------
// 캐릭터 이동시작 패킷						Client -> Server
//
// 자신의 캐릭터 이동시작시 이 패킷을 보낸다.
// 이동 중에는 본 패킷을 보내지 않으며, 키 입력이 변경되었을 경우에만
// 보내줘야 한다.
//
// (왼쪽 이동중 위로 이동 / 왼쪽 이동중 왼쪽 위로 이동... 등등)
//---------------------------------------------------------------
struct	packet_cs_move_start {
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_MOVE_START					11
//---------------------------------------------------------------
// 캐릭터 이동시작 패킷						Server -> Client
//
// 다른 유저의 캐릭터 이동시 본 패킷을 받는다.
// 패킷 수신시 해당 캐릭터를 찾아 이동처리를 해주도록 한다.
// 
// 패킷 수신 시 해당 키가 계속해서 눌린것으로 생각하고
// 해당 방향으로 계속 이동을 하고 있어야만 한다.
//---------------------------------------------------------------
struct	packet_sc_move_start {
	unsigned int id;
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_CS_MOVE_STOP					12
//---------------------------------------------------------------
// 캐릭터 이동중지 패킷						Client -> Server
//
// 이동중 키보드 입력이 없어서 정지되었을 때, 이 패킷을 서버에 보내준다.
// 이동중 방향 전환시에는 스탑을 보내지 않는다.
//---------------------------------------------------------------
struct	packet_cs_move_stop {
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_MOVE_STOP					13
//---------------------------------------------------------------
// 캐릭터 이동중지 패킷						Server -> Client
//
// ID 에 해당하는 캐릭터가 이동을 멈춘것이므로 
// 캐릭터를 찾아서 방향과, 좌표를 입력해주고 멈추도록 처리한다.
//---------------------------------------------------------------
struct	packet_sc_move_stop {
	unsigned int id;
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_CS_ATTACK1						20
//---------------------------------------------------------------
// 캐릭터 공격 패킷							Client -> Server
//
// 공격 키 입력시 본 패킷을 서버에게 보낸다.
// 충돌 및 데미지에 대한 결과는 서버에서 알려 줄 것이다.
//
// 공격 동작 시작시 한번만 서버에게 보내줘야 한다.
//---------------------------------------------------------------
struct	packet_cs_attack1 {
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_ATTACK1						21
//---------------------------------------------------------------
// 캐릭터 공격 패킷							Server -> Client
//
// 패킷 수신시 해당 캐릭터를 찾아서 공격1번 동작으로 액션을 취해준다.
// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
//---------------------------------------------------------------
struct	packet_sc_attack1 {
	unsigned int id;
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_CS_ATTACK2						22
//---------------------------------------------------------------
// 캐릭터 공격 패킷							Client -> Server
//
// 공격 키 입력시 본 패킷을 서버에게 보낸다.
// 충돌 및 데미지에 대한 결과는 서버에서 알려 줄 것이다.
//
// 공격 동작 시작시 한번만 서버에게 보내줘야 한다.
//---------------------------------------------------------------
struct	packet_cs_attack2 {
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_ATTACK2					23
//---------------------------------------------------------------
// 캐릭터 공격 패킷							Server -> Client
//
// 패킷 수신시 해당 캐릭터를 찾아서 공격2번 동작으로 액션을 취해준다.
// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
//---------------------------------------------------------------
struct	packet_sc_attack2 {
	unsigned int id;
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_CS_ATTACK3						24
//---------------------------------------------------------------
// 캐릭터 공격 패킷							Client -> Server
//
// 공격 키 입력시 본 패킷을 서버에게 보낸다.
// 충돌 및 데미지에 대한 결과는 서버에서 알려 줄 것이다.
//
// 공격 동작 시작시 한번만 서버에게 보내줘야 한다.
//---------------------------------------------------------------
struct	packet_cs_attack3 {
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_ATTACK3					25
//---------------------------------------------------------------
// 캐릭터 공격 패킷							Server -> Client
//
// 패킷 수신시 해당 캐릭터를 찾아서 공격2번 동작으로 액션을 취해준다.
// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
//---------------------------------------------------------------
struct	packet_sc_attack3 {
	unsigned int id;
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_DAMAGE						30
//---------------------------------------------------------------
// 캐릭터 데미지 패킷							Server -> Client
//
// 공격에 맞은 캐릭터의 정보를 보냄.
//
//	4	-	AttackID	( 공격자 ID )
//	4	-	DamageID	( 피해자 ID )
//	1	-	DamageHP	( 피해자 HP )
//
//---------------------------------------------------------------
struct	packet_sc_damage {
	unsigned int attacker_id;
	unsigned int damaged_id;
	char hp;
};

//#define	PACKET_CS_SYNC						250 // 사용안함...
//---------------------------------------------------------------
// 동기화를 위한 패킷					Client -> Server
//---------------------------------------------------------------
struct	packet_cs_sync {
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_SYNC						251
//---------------------------------------------------------------
// 동기화를 위한 패킷					Server -> Client
//
// 서버로부터 동기화 패킷을 받으면 해당 캐릭터를 찾아서
// 캐릭터 좌표를 보정해준다.
//---------------------------------------------------------------
struct	packet_sc_sync {
	unsigned int id;
	unsigned short x;
	unsigned short y;
};

#pragma pack(pop)