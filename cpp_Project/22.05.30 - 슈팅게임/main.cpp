#include "stdafx.h"
#include "Console.h"
#include "ScreenBuffer.h"
#include "DataFileIO.h"
#include "GameManager.h"

/*
�÷��� ���
1. ����Ű�� ����Ͽ� �÷��̾� �̵�
2. �����̽� ��ư �Է����� �Ѿ˹߻�

* ESCŰ�� ������ �������� ��ŵ���� (���� ���������� ��ȯ)
  TITLE, CLEAR, OVER ȭ�鿡�� ENTERŰ�� ������ ����ȯ // TITLE -> STAGE, CLEAR / OVER - > TITLE

���� ���� (���� ������?)
	EnemyInfo.txt
		1. ù��°���� ������ �� ������ �ִ��� ��Ÿ��
		2. �ι�° �ٺ��ʹ� ������ ������ ������ ��Ÿ��
		3. �ش� ������ �����ؼ� ���� ������ �ø� �� ���� // 1��° �ٿ� ��õȸ�ŭ�� ���������� ����ؾ���
	Enemy.txt
		1. ù���� �ش� Enemy�� CHAR�� ��Ÿ��
		2. �ι�°���� ������ ��Ÿ�� // *�� �Ѿ˹߻�, 0~7 �� �̵� (0�� N, 1�� NE ~ 7�� NW�� �ð�������� 8������ ǥ��)
		3. �ش� ������ �����ؼ� ���� CHAR, ������ ���� �� ����������

	TITLE, CLEAR, OVER
		�ش� ���� �׷��� ������
*/

int main(void)
{
	game_manager_inst().Init();
	game_manager_inst().StartGame();
}