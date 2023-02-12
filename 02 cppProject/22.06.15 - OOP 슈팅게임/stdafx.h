#include <windows.h>
#include <iostream>
#include <memory.h>
#include <conio.h>
#include <vector>
#include <list>
//#include "j_new.h"

#define dfSCREEN_WIDTH			81		// ÄÜ¼Ö °¡·Î 80Ä­ + NULL
#define dfSCREEN_HEIGHT			24		// ÄÜ¼Ö ¼¼·Î 24Ä­
#define SCREEN_SIZE				dfSCREEN_WIDTH * dfSCREEN_HEIGHT

#define MAX_INFO_SIZE			128
#define MAX_STAGE				64
#define MAX_ENEMY_TYPE			64
#define MAX_KIND_ENEMY			32
#define MAX_ENEMY_PATTERN		128
#define MAX_ENEMY_FILE_SIZE		MAX_ENEMY_PATTERN + 16

#define PLAYER_CHAR 'A'

using namespace std;

#define TITLE_FILE		".\\GamaData\\TITLE.txt"
#define STAGEINFO_FILE	".\\GamaData\\StageInfo.txt"
#define ENEMYINFO_FILE	".\\GamaData\\EnemyInfo.txt"
#define OVER_FILE		".\\GamaData\\GAMEOVER.txt"
#define CLEAR_FILE		".\\GamaData\\Clear.txt"

#include "j_list.h"
#define list j_list