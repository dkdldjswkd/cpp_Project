#include "stdafx.h"
#include "BinaryTree.h"
#include <conio.h>
#include "WndProc.h"
using namespace std;

#define RADIUS 30
#define RECT_SIZE 20

// ��Ʈ ���� ������ �̾�� �Ѵ�.
// ���� ���� ������ �̾�� �Ѵ�.
// ���� ����� �ڽ��� ������ �̾�� �Ѵ�
// ��Ʈ ���� ���� ��� ���� �������� �� ����� ������ ���ƾ� �Ѵ�.

int main() {
	system(" mode  con lines=20   cols=50 ");
	setlocale(LC_ALL, "korean");

	Init_Window();

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}