#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>

using namespace std;

#define SECTOR_MAX_X 16
#define SECTOR_MAX_Y 16

struct Player {
	SRWLOCK lock;
	bool move = false;

	int x = 8;
	int y = 8;
};

Player playerArr[10];

SRWLOCK sector_lock[SECTOR_MAX_Y][SECTOR_MAX_X];
std::unordered_set<Player*> sectors_set[SECTOR_MAX_Y][SECTOR_MAX_X];

#define DIR_RIGHT	0
#define DIR_LEFT	1
#define DIR_DOWN	2
#define DIR_UP		3

int g_v = 0;
void f() {
	auto v = InterlockedIncrement((LONG*)&g_v);

	for (;;) {
		for (int i = 0; i < v-1; i ++ ) {
			printf("  ");
		}
		printf("%d \n", v);

		auto player_index = rand() % 10; // 플레이어 아무나 뽑기
		if (playerArr[player_index].move == true) continue; // 이동 중
		if (InterlockedExchange8((CHAR*)&playerArr[player_index].move, true) == true) continue; // 이동 중

		Player* p_player = &playerArr[player_index];

		// 이동방향 결정
		int dir;
		if (rand() % 2 == 0) { // 수평 이동
			if (p_player->x == 0)
				dir = DIR_RIGHT;
			else if(p_player->x == SECTOR_MAX_X -1)
				dir = DIR_LEFT;
			else 
				dir = rand() % 2;
		}
		else { // 수직 이동
			if (p_player->y == 0)
				dir = DIR_DOWN;
			else if (p_player->y == SECTOR_MAX_Y - 1)
				dir = DIR_UP;
			else 
				dir = (rand() % 2) + 2;
		}

		// 이동
		switch (dir) {
		case DIR_RIGHT: {


			AcquireSRWLockExclusive(&sector_lock[p_player->y][p_player->x]);
			AcquireSRWLockExclusive(&sector_lock[p_player->y][p_player->x + 1]);

			sectors_set[p_player->y][p_player->x].erase(p_player);
			sectors_set[p_player->y][p_player->x + 1].insert(p_player);

			ReleaseSRWLockExclusive(&sector_lock[p_player->y][p_player->x + 1]);
			ReleaseSRWLockExclusive(&sector_lock[p_player->y][p_player->x]);
			break;
		}

		case DIR_LEFT: {
			AcquireSRWLockExclusive(&sector_lock[p_player->y][p_player->x - 1]);
			AcquireSRWLockExclusive(&sector_lock[p_player->y][p_player->x]);

			sectors_set[p_player->y][p_player->x].erase(p_player);
			sectors_set[p_player->y][p_player->x - 1].insert(p_player);

			ReleaseSRWLockExclusive(&sector_lock[p_player->y][p_player->x]);
			ReleaseSRWLockExclusive(&sector_lock[p_player->y][p_player->x - 1]);
			break;
		}

		case DIR_DOWN: {
			AcquireSRWLockExclusive(&sector_lock[p_player->y + 1][p_player->x]);
			AcquireSRWLockExclusive(&sector_lock[p_player->y][p_player->x]);

			sectors_set[p_player->y][p_player->x].erase(p_player);
			sectors_set[p_player->y + 1][p_player->x].insert(p_player);

			ReleaseSRWLockExclusive(&sector_lock[p_player->y][p_player->x]);
			ReleaseSRWLockExclusive(&sector_lock[p_player->y + 1][p_player->x]);
			break;
		}

		case DIR_UP: {
			AcquireSRWLockExclusive(&sector_lock[p_player->y][p_player->x]);
			AcquireSRWLockExclusive(&sector_lock[p_player->y - 1][p_player->x]);

			sectors_set[p_player->y][p_player->x].erase(p_player);
			sectors_set[p_player->y - 1][p_player->x].insert(p_player);

			ReleaseSRWLockExclusive(&sector_lock[p_player->y - 1][p_player->x]);
			ReleaseSRWLockExclusive(&sector_lock[p_player->y][p_player->x]);
			break;
		}
		}

		InterlockedExchange8((CHAR*)&playerArr[player_index].move, false);
	}
}

void f2() {
	auto v = InterlockedIncrement((LONG*)&g_v);
	for (;;) {
		for (int i = 0; i < v - 1; i++) {
			printf("  ");
		}
		printf("%d \n", v);

		int x = rand() % 16;
		int y = rand() % 16;

		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				auto tmp_x = x + i;
				auto tmp_y = y + j;

				if (tmp_x <= -1) continue;
				else if (tmp_x >= SECTOR_MAX_X) continue;
				else if (tmp_y <= -1) continue;
				else if (tmp_y >= SECTOR_MAX_Y) continue;
				AcquireSRWLockShared(&sector_lock[i][j]);
				Sleep(0);
			}
		}

		for (int i = 1; i >= -1; i--) {
			for (int j = 1; j >= -1; j--) {
				auto tmp_x = x + i;
				auto tmp_y = y + j;

				if (tmp_x <= -1) continue;
				else if (tmp_x >= SECTOR_MAX_X) continue;
				else if (tmp_y <= -1) continue;
				else if (tmp_y >= SECTOR_MAX_Y) continue;
				ReleaseSRWLockShared(&sector_lock[i][j]);
			}
		}
	}
}

int main() {
	// 초기화
	for (int i = 0; i < 10; i++) {
		InitializeSRWLock(&playerArr[i].lock);
		sectors_set[playerArr[i].y][playerArr[i].x].insert(&playerArr[i]);
	}

	for (int i = 0; i < SECTOR_MAX_Y; i++) {
		for (int j = 0; j < SECTOR_MAX_X; j++) {
			InitializeSRWLock(&sector_lock[i][j]);
		}
	}

	thread a(f);
	thread b(f);
	thread c(f);
	thread d(f2);
	thread e(f2);
	Sleep(INFINITE);
}