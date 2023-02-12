#include "stdafx.h"
#include "Server.h"
#include "Log.h"
#include "Define.h"

bool shutdown_server = false;

int session_count		= 0;
int character_count		= 0;
int syncMsg_count		= 0;
int moveStart_sync		= 0;
int moveStop_sync		= 0;
int attack_sync			= 0;
int sessionPool_count	= 0;
int characterPool_count = 0;
int fin_count			= 0;
int disconnect_count	= 0;

void Monitor(DWORD t);


unsigned loop_count = 0;
unsigned logic_count = 0;
int total_loop = 0;

int main() {
	timeBeginPeriod(1);

	//------------------------------
	// Init
	//------------------------------

	// �α�
	Log::SetThreshold(Log::LOG_TYPE_DEBUG);
	Log::Initialise("MMO LOG.txt");

	// ��Ʈ��ũ
	Network::StartUp();

	// loop timer
	DWORD cur_time = timeGetTime();

	while (!shutdown_server) {
		total_loop++;

		//------------------------------
		// time
		//------------------------------
		loop_count++;
		DWORD prev_time		= cur_time;						// prev time �ֽ�ȭ
		cur_time			= timeGetTime();				// cur time �ֽ�ȭ
		DWORD delta_time	= cur_time - prev_time;			// get delta time

		//------------------------------
		// ����͸�
		//------------------------------
		Monitor(delta_time);

		//------------------------------
		// ��Ʈ��ũ
		//------------------------------
		Network::Process_NetIO();

		//------------------------------
		// ����
		//------------------------------
		Update(delta_time);

		//------------------------------
		// ���� ����
		//------------------------------
		Network::Disconnect_Session();
	}

	// ���ҽ� ���� (ex. DB ������ ���� Ȯ��)
	Network::CleanUp();
	return 0;
}

// 1�ʿ� �ѹ�
DWORD monitor_timer = 0;
void Monitor(DWORD t) {
	monitor_timer += t;

	if (monitor_timer >= 1000)
		monitor_timer = 0;
	else
		return;

	static DWORD server_run_time = 0;

// CONSOLE I/O
printf("\
loop  frame     : %d \n\
logic frame     : %d \n\
server run time : %u \n\
			    	 \n\
session         : %d \n\
character       : %d \n\
disconnect      : %d \n\
			    	 \n\
-- SYNC --			 \n\
Move Start Sync : %d \n\
Move Stop Sync  : %d \n\
Attack Sync     : %d \n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
",
loop_count,						// ������
logic_count,					// ���� ������
server_run_time++,				// ���� ���� �� �帥 �ð�(��)
session_count,					// ���� ��
character_count,				// ĳ���� ��
disconnect_count,				// �� ������ ���� ��
moveStart_sync,					// move start sync
moveStop_sync,					// move stop sync
attack_sync					// attack sync
);

if (loop_count < FRAME_PER_SEC)
	Log::Fatal("!! [Frame Drop] %d ", logic_count);
if (logic_count < FRAME_PER_SEC)
	Log::Warn("[Frame Drop] %d ", logic_count);

// ī��Ʈ �ʱ�ȭ
loop_count = 0;
logic_count = 0;
}