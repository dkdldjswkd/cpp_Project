#include <windows.h>
#include <iostream>
#include <memory.h>
#include <conio.h>
#include <vector>
#include <list>
using namespace std;

#define dfSCREEN_WIDTH		81		// 콘솔 가로 80칸 + NULL
#define dfSCREEN_HEIGHT		24		// 콘솔 세로 24칸

#define TITLE_FILE_NAME ".\\GamaData\\TITLE.txt"
#define STAGEINFO_FILE_NAME ".\\GamaData\\StageInfo.txt"
#define ENEMYINFO_FILE_NAME ".\\GamaData\\EnemyInfo.txt"
#define CLEAR_FILE_NAME ".\\GamaData\\Clear.txt"
#define OVER_FILE_NAME ".\\GamaData\\GAMEOVER.txt"

bool Pos_Check(int x, int y);