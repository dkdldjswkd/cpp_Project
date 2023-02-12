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

	// 로그
	Log::SetThreshold(Log::LOG_TYPE_DEBUG);
	Log::Initialise("MMO LOG.txt");

	// 네트워크
	Network::StartUp();

	// loop timer
	DWORD cur_time = timeGetTime();

	while (!shutdown_server) {
		total_loop++;

		//------------------------------
		// time
		//------------------------------
		loop_count++;
		DWORD prev_time		= cur_time;						// prev time 최신화
		cur_time			= timeGetTime();				// cur time 최신화
		DWORD delta_time	= cur_time - prev_time;			// get delta time

		//------------------------------
		// 모니터링
		//------------------------------
		Monitor(delta_time);

		//------------------------------
		// 네트워크
		//------------------------------
		Network::Process_NetIO();

		//------------------------------
		// 로직
		//------------------------------
		Update(delta_time);

		//------------------------------
		// 세션 끊기
		//------------------------------
		Network::Disconnect_Session();
	}

	// 리소스 정리 (ex. DB 데이터 저장 확인)
	Network::CleanUp();
	return 0;
}

// 1초에 한번
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
loop_count,						// 프레임
logic_count,					// 로직 프레임
server_run_time++,				// 서버 시작 후 흐른 시간(초)
session_count,					// 세션 수
character_count,				// 캐릭터 수
disconnect_count,				// 총 끊어진 세션 수
moveStart_sync,					// move start sync
moveStop_sync,					// move stop sync
attack_sync					// attack sync
);

if (loop_count < FRAME_PER_SEC)
	Log::Fatal("!! [Frame Drop] %d ", logic_count);
if (logic_count < FRAME_PER_SEC)
	Log::Warn("[Frame Drop] %d ", logic_count);

// 카운트 초기화
loop_count = 0;
logic_count = 0;
}