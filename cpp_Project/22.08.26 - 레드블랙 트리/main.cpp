#include "stdafx.h"
#include "BinaryTree.h"
#include <conio.h>
#include "WndProc.h"
using namespace std;

#define RADIUS 30
#define RECT_SIZE 20

// 루트 노드는 검은색 이어야 한다.
// 리프 노드는 검은색 이어야 한다.
// 레드 노드의 자식은 검은색 이어야 한다
// 루트 노드로 부터 모든 리프 노드까지의 블랙 노드의 개수가 같아야 한다.

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