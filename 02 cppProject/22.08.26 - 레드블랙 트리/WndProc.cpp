#include "stdafx.h"
#include "WndProc.h"
#include "BinaryTree.h"
#include <string>
#include <conio.h>

#define DEBUG_TREE
#define DEBUF_LOOP	30
#define MAX_NODE_SIZE 300
#define MAX_DATA_RANGE (MAX_NODE_SIZE *5)
#define RAND_NUM	(rand()  % MAX_DATA_RANGE)

using namespace std;

MSG	msg;
HWND h_wnd;

RedBlackTree tree;
int node_size = 0;

wchar_t input_order;
wchar_t input_data[64];
bool flag_input_data = false;
int tree_data = 0;
int order;

bool Set_Order(wchar_t wchar) {
	// 엔터
	if (wchar == 13) {
		// 데이터 입력 모드에서 엔터 입력 시
		if (flag_input_data) {
			if(0  < wcslen(input_data))
				tree_data = stoi(input_data);

			order = input_order - 0x30;
			input_order = 0;
			input_data[0] = 0;

			flag_input_data = false;
			return true;
		}
		// 명령 입력 모드에서 엔터 입력 시
		else {
			// 명령은 1 ~ 4 까지만 되게
			if (input_order < 0x31 || 0x34 < input_order)
				return false;

			flag_input_data = true;
			return false;
		}
	}

	// 숫자만 입력받음
	if (wchar < 0x30 || 0x39 < wchar)
		return false;

	// 명령 입력
	if (!flag_input_data) {
		input_order = wchar;
	}
	// 데이터 입력
	else {
		int len = wcslen(input_data);
		if (7 < len) // 백만 단위까지만 허용
			return false;

		input_data[len] = wchar;
		input_data[len + 1] = 0;
	}

	return false;
}

void write_window() {
	// 출력 데이터 셋팅
	//wchar_t msg[64] = L"(1) 삽입, (2) 삭제, (3) 랜덤 삽입, (4) 랜덤 삭제, (5) 검증, (6) 무한검증 >> ";
	wchar_t msg[64] = L"(1) 삽입, (2) 삭제, (3) 랜덤 삽입, (4) 랜덤 삭제 >> ";

	int msg_size = wcslen(msg);
	msg[msg_size] = input_order;
	msg[msg_size + 1] = 0;

	wchar_t buf[64] = L"";
	wchar_t msg2[64] = L"데이터 입력 >> ";

	if(flag_input_data)
		wsprintf(buf, L"%s%s", msg2, input_data);

	int x = 220;
	int y = 800;

	int rect_x_size = 300;
	int rect_y_size = 100;

	// 윈도우 출력 (DC 사용 부분)
	HDC hdc;
	PAINTSTRUCT ps;
	hdc = GetDC(h_wnd);

	RECT rect = { x - rect_x_size,y - rect_y_size,x + rect_x_size,y + rect_y_size };
	DrawTextW(hdc, msg, -1, &rect, DT_CENTER | DT_WORDBREAK);

	RECT rect2 = { x - rect_x_size, y + 50 - rect_y_size, x + rect_x_size, y + 50 + rect_y_size };
	DrawTextW(hdc, buf, -1, &rect2, DT_CENTER | DT_WORDBREAK);

	ReleaseDC(h_wnd, hdc);
}

void Verific() {
	// 모든 경우의 블랙 수
	tree.Check_Black();
	// 레드레드 없는지 검사
	tree.Son_is_black();
	// 루트 노드 레드 아닌지 검사
	// 루트 노드가 존재한다면, (노드 개수가 1 이상이라면)
	if (tree.nill_node.left != &tree.nill_node) {
		if (tree.nill_node.left->red == RED) {
			throw;
		}
	}
	// 닐노드 레드 아닌지 검사
	if (tree.nill_node.red == RED) {
		throw;
	}
}

void Rand_Input() {
	if (MAX_NODE_SIZE - MAX_NODE_SIZE/10 < node_size)
		return;

	// 하나 넣을때까지 뺑뺑이
	for (;;) {
		int n = RAND_NUM;
		if (tree.Insert(n)) {
			//printf("랜덤 입력 : %d \n", n);
			break;
		}
	}

	node_size++;
}

void Rand_Delete() {
	if (node_size < 1)
		return;

	// 하나 지울때까지 뺑뺑이
	for (;;) {
		auto n = RAND_NUM;
		if (tree.Remove_Node(n)) {
			//printf("랜덤 삭제 : %d \n", n);
			break;
		}
	}

	node_size--;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM w_param, LPARAM l_param){
	switch (message) {

	case WM_KEYDOWN: {
		// 엔터
		if (Set_Order(w_param)) {
			switch (order) {
				// INPUT
			case 1: {
				printf("입력 : %d \n", tree_data);
				tree.Insert(tree_data);
				break;
			}

					 // DELETE NODE
			case 2: {
				printf("삭제 : %d \n", tree_data);
				tree.Remove_Node(tree_data);
				break;
			}

				  // RANDOM INPUT
			case 3: {
				Rand_Input();
				break;
			}

				  // RANDOM DELETE
			case 4: {
				Rand_Delete();
				break;
			}

					 // 디버깅
			case 5: {
				//cout << "블랙 노드 개수 : " << tree.Number_of_black() << endl;
				Verific();

				break;
			}

				  // 무한 디버깅
			case 6: {

				for (;;) {
					// 최대 MAX_NODE_SIZE 만큼 랜덤 삽입
					int n = rand() % (MAX_NODE_SIZE - node_size);

					for (int i = 0; i < n; i++) {
						Rand_Input();
						Verific();
					}

					cout << "node_size : " << node_size << ", " << n << "개 삽입" << endl;

					// 디버깅
					draw_white();
					tree.Print();

					// 최대 MAX_NODE_SIZE 만큼 랜덤 삭제
					n = rand() % node_size;

					for (int i = 0; i < n; i++) {
						Rand_Delete();
						Verific();
					}

					cout << "node_size : " << node_size << ", " << n << "개 삭제" << endl;
					// 디버깅
					draw_white();
					tree.Print();

					// 무한루프 탈출
					if (_kbhit()) {
						int aaa;
						cin >> aaa;
						if (aaa == 1)
							break;
					}
				}


				break;
			}

			default:
				break;
			}
		}

		InvalidateRect(h_wnd, nullptr, TRUE);
		break;
	}

	case WM_PAINT: {
		PAINTSTRUCT	ps;
		HDC	hdc;
		hdc = BeginPaint(h_wnd, &ps);

		tree.Print();
		write_window();

		EndPaint(h_wnd, &ps);
		break;
	}

	case WM_DESTROY:
		PostQuitMessage(100);
		break;

	default:
		return DefWindowProc(hWnd, message, w_param, l_param);
	}

	return 0;
}

void Init_Window() {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = TEXT(CLASS_NAME);
	wcex.hIconSm = NULL;
	RegisterClassEx(&wcex);

	h_wnd = CreateWindowW(TEXT(CLASS_NAME),
		TEXT(TITLE_NAME),
		WS_OVERLAPPEDWINDOW,
		650, 100, // 생성 위치
		WINDOW_WIDTH + WINDOW_WIDTH / 10, WINDOW_HEIGHT + WINDOW_HEIGHT / 10, // 사이즈
		NULL,
		NULL,
		NULL,
		NULL);

	ShowWindow(h_wnd, SW_SHOWNORMAL);
	UpdateWindow(h_wnd);
}