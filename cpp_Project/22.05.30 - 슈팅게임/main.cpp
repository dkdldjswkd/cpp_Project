#include "stdafx.h"
#include "Console.h"
#include "ScreenBuffer.h"
#include "DataFileIO.h"
#include "GameManager.h"

/*
플레이 방법
1. 방향키를 사용하여 플레이어 이동
2. 스페이스 버튼 입력으로 총알발사

* ESC키를 눌러서 스테이지 스킵가능 (다음 스테이지로 전환)
  TITLE, CLEAR, OVER 화면에서 ENTER키를 눌러서 씬전환 // TITLE -> STAGE, CLEAR / OVER - > TITLE

구현 사항 (게임 편집기?)
	EnemyInfo.txt
		1. 첫번째줄은 적들이 몇 종류가 있는지 나타냄
		2. 두번째 줄부터는 적들의 데이터 파일을 나타냄
		3. 해당 파일을 수정해서 적의 종류를 늘릴 수 있음 // 1번째 줄에 명시된만큼의 적군파일을 명시해야함
	Enemy.txt
		1. 첫줄은 해당 Enemy의 CHAR를 나타냄
		2. 두번째줄은 패턴을 나타냄 // *은 총알발사, 0~7 은 이동 (0은 N, 1은 NE ~ 7은 NW로 시계방향으로 8방향을 표현)
		3. 해당 파일을 수정해서 적의 CHAR, 움직임 패턴 등 수정가능함

	TITLE, CLEAR, OVER
		해당 씬에 그려질 데이터
*/

int main(void)
{
	game_manager_inst().Init();
	game_manager_inst().StartGame();
}