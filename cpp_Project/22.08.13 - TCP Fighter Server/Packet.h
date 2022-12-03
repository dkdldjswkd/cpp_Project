#pragma once
#include "PacketDefine.h"

// �̵� ����üũ ����
#define ERROR_RANGE		50

// ȭ�� �̵�����, �����Ӵ� �̵� ���� -  X : 3, Y : 2
#define RANGE_MOVE_TOP		50
#define RANGE_MOVE_BOTTOM	470
#define RANGE_MOVE_LEFT		10
#define RANGE_MOVE_RIGHT	630

// ���ݹ���
#define ATTACK1_RANGE_X		80
#define ATTACK1_RANGE_Y		10

#define ATTACK2_RANGE_X		90
#define ATTACK2_RANGE_Y		10

#define ATTACK3_RANGE_X		100
#define ATTACK3_RANGE_Y		20

// ������
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
	unsigned char code = 0x89;	// ��Ŷ�ڵ� 0x89 ����
	unsigned char size = 0;			// ��Ŷ ������
	unsigned char type = ~0;			// ��Ŷ Ÿ��
};

// #define PACKET_SC_CREATE_MY_CHARACTER			0
//---------------------------------------------------------------
// Ŭ���̾�Ʈ �ڽ��� ĳ���� �Ҵ�		Server -> Client
//
// ������ ���ӽ� ���ʷ� �ްԵǴ� ��Ŷ���� �ڽ��� �Ҵ���� ID ��
// �ڽ��� ���� ��ġ, HP �� �ް� �ȴ�. (ó���� �ѹ� �ް� ��)
// 
// �� ��Ŷ�� ������ �ڽ��� ID,X,Y,HP �� �����ϰ� ĳ���͸� �������Ѿ� �Ѵ�.
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
// �ٸ� Ŭ���̾�Ʈ�� ĳ���� ���� ��Ŷ		Server -> Client
//
// ó�� ������ ���ӽ� �̹� ���ӵǾ� �ִ� ĳ���͵��� ����
// �Ǵ� �����߿� ���ӵ� Ŭ���̾�Ʈ���� ������ ����.
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
// ĳ���� ���� ��Ŷ						Server -> Client
//
// ĳ������ �������� �Ǵ� ĳ���Ͱ� �׾����� ���۵�.
//---------------------------------------------------------------
struct	packet_sc_delete_character {
	unsigned int id;
};

//#define	PACKET_CS_MOVE_START					10
//---------------------------------------------------------------
// ĳ���� �̵����� ��Ŷ						Client -> Server
//
// �ڽ��� ĳ���� �̵����۽� �� ��Ŷ�� ������.
// �̵� �߿��� �� ��Ŷ�� ������ ������, Ű �Է��� ����Ǿ��� ��쿡��
// ������� �Ѵ�.
//
// (���� �̵��� ���� �̵� / ���� �̵��� ���� ���� �̵�... ���)
//---------------------------------------------------------------
struct	packet_cs_move_start {
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_MOVE_START					11
//---------------------------------------------------------------
// ĳ���� �̵����� ��Ŷ						Server -> Client
//
// �ٸ� ������ ĳ���� �̵��� �� ��Ŷ�� �޴´�.
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�� �̵�ó���� ���ֵ��� �Ѵ�.
// 
// ��Ŷ ���� �� �ش� Ű�� ����ؼ� ���������� �����ϰ�
// �ش� �������� ��� �̵��� �ϰ� �־�߸� �Ѵ�.
//---------------------------------------------------------------
struct	packet_sc_move_start {
	unsigned int id;
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_CS_MOVE_STOP					12
//---------------------------------------------------------------
// ĳ���� �̵����� ��Ŷ						Client -> Server
//
// �̵��� Ű���� �Է��� ��� �����Ǿ��� ��, �� ��Ŷ�� ������ �����ش�.
// �̵��� ���� ��ȯ�ÿ��� ��ž�� ������ �ʴ´�.
//---------------------------------------------------------------
struct	packet_cs_move_stop {
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_MOVE_STOP					13
//---------------------------------------------------------------
// ĳ���� �̵����� ��Ŷ						Server -> Client
//
// ID �� �ش��ϴ� ĳ���Ͱ� �̵��� ������̹Ƿ� 
// ĳ���͸� ã�Ƽ� �����, ��ǥ�� �Է����ְ� ���ߵ��� ó���Ѵ�.
//---------------------------------------------------------------
struct	packet_sc_move_stop {
	unsigned int id;
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_CS_ATTACK1						20
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Client -> Server
//
// ���� Ű �Է½� �� ��Ŷ�� �������� ������.
// �浹 �� �������� ���� ����� �������� �˷� �� ���̴�.
//
// ���� ���� ���۽� �ѹ��� �������� ������� �Ѵ�.
//---------------------------------------------------------------
struct	packet_cs_attack1 {
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_ATTACK1						21
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Server -> Client
//
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�Ƽ� ����1�� �������� �׼��� �����ش�.
// ������ �ٸ� ��쿡�� �ش� �������� �ٲ� �� ���ش�.
//---------------------------------------------------------------
struct	packet_sc_attack1 {
	unsigned int id;
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_CS_ATTACK2						22
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Client -> Server
//
// ���� Ű �Է½� �� ��Ŷ�� �������� ������.
// �浹 �� �������� ���� ����� �������� �˷� �� ���̴�.
//
// ���� ���� ���۽� �ѹ��� �������� ������� �Ѵ�.
//---------------------------------------------------------------
struct	packet_cs_attack2 {
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_ATTACK2					23
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Server -> Client
//
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�Ƽ� ����2�� �������� �׼��� �����ش�.
// ������ �ٸ� ��쿡�� �ش� �������� �ٲ� �� ���ش�.
//---------------------------------------------------------------
struct	packet_sc_attack2 {
	unsigned int id;
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_CS_ATTACK3						24
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Client -> Server
//
// ���� Ű �Է½� �� ��Ŷ�� �������� ������.
// �浹 �� �������� ���� ����� �������� �˷� �� ���̴�.
//
// ���� ���� ���۽� �ѹ��� �������� ������� �Ѵ�.
//---------------------------------------------------------------
struct	packet_cs_attack3 {
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_ATTACK3					25
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Server -> Client
//
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�Ƽ� ����2�� �������� �׼��� �����ش�.
// ������ �ٸ� ��쿡�� �ش� �������� �ٲ� �� ���ش�.
//---------------------------------------------------------------
struct	packet_sc_attack3 {
	unsigned int id;
	unsigned char dir;
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_DAMAGE						30
//---------------------------------------------------------------
// ĳ���� ������ ��Ŷ							Server -> Client
//
// ���ݿ� ���� ĳ������ ������ ����.
//
//	4	-	AttackID	( ������ ID )
//	4	-	DamageID	( ������ ID )
//	1	-	DamageHP	( ������ HP )
//
//---------------------------------------------------------------
struct	packet_sc_damage {
	unsigned int attacker_id;
	unsigned int damaged_id;
	char hp;
};

//#define	PACKET_CS_SYNC						250 // ������...
//---------------------------------------------------------------
// ����ȭ�� ���� ��Ŷ					Client -> Server
//---------------------------------------------------------------
struct	packet_cs_sync {
	unsigned short x;
	unsigned short y;
};

//#define	PACKET_SC_SYNC						251
//---------------------------------------------------------------
// ����ȭ�� ���� ��Ŷ					Server -> Client
//
// �����κ��� ����ȭ ��Ŷ�� ������ �ش� ĳ���͸� ã�Ƽ�
// ĳ���� ��ǥ�� �������ش�.
//---------------------------------------------------------------
struct	packet_sc_sync {
	unsigned int id;
	unsigned short x;
	unsigned short y;
};

#pragma pack(pop)