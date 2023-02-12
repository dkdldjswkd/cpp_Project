#include <Windows.h>
#include <windowsx.h>
#include <iostream>
#include <map>
#include <list>
#include <algorithm>
#include "Pos.h"
using namespace std;

#define CLASS_NAME "class name"
#define TITLE_NAME "title name"

// 윈도우 사이즈, 시작 위치
#define WINDOW_WIDTH	1650
#define WINDOW_HEIGHT	850
#define CREATE_POS_X	100
#define CREATE_POS_Y	100

// 그리드 정보
#define GRID_SIZE 16
#define GRID_WIDTH 100
#define GRID_HEIGHT 50

#define NONE 0
#define OBSTACLE 1
#define START_INDEX 2
#define END_INDEX 3
#define VISITED 4
#define SCHEDULED 5

struct Node {
	Pos pos;

	float g; // 출발점에서의 이동거리
	short h; // 목적지와의 거리
	float f; // g + h, f가 가장 작은 노드를 찾으며 퍼저나간다.
	Node* parent = nullptr;

public:
	void Set(Pos pos, float g, short h, Node* parent) {
		this->pos = pos;
		this->g = g;
		this->h = h;
		f = g + h;
		this->parent = parent;
	}
};
Node nodes[GRID_HEIGHT][GRID_WIDTH];

// 0 장애물 없음, 1 장애물 있음, 2 start point, 3 end point
char tile_type[GRID_HEIGHT][GRID_WIDTH] = { 0, };

Pos index_start{ 0,0 };
Pos index_prev_start{ 0,0 };

Pos index_end{ GRID_WIDTH - 1, GRID_HEIGHT - 1 };
Pos index_prev_end{ GRID_WIDTH - 1, GRID_HEIGHT - 1 };

enum DRAW_TYPE {
	DRAW_OBSTACLE,
	ERASE,
	MOVE_START,
	MOVE_END,
};

DRAW_TYPE draw_type;

bool flag_drag = false;

HBRUSH g_hTileBrush;
HPEN g_hGridPen;

void Init_tile(Pos start = { 0,0 }, Pos end = { GRID_WIDTH -1,GRID_HEIGHT - 1 }) {
	index_start = start;
	index_end = end;

	tile_type[index_start.y][index_start.x] = START_INDEX;
	tile_type[index_end.y][index_end.x] = END_INDEX;
}

void Init_consol() {
	system(" mode  con lines=20   cols=50 ");
}

HWND h_wnd;
MSG msg;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL Init_window();
void RenderGrid(HDC hdc);

void RenderObstacle(HDC	hdc) {
	HBRUSH	HOldBrush = (HBRUSH)SelectObject(hdc, g_hTileBrush);
	SelectObject(hdc, GetStockObject(NULL_PEN));

	Pos pos_rect(0, 0);
	Pos index(0, 0);
	for (index.x = 0; index.x < GRID_WIDTH; index.x++) {
		for (index.y = 0; index.y < GRID_HEIGHT; index.y++) {
			switch (tile_type[index.y][index.x])
			{
			case OBSTACLE:
			{
				pos_rect.x = index.x * GRID_SIZE;
				pos_rect.y = index.y * GRID_SIZE;
				Rectangle(hdc, pos_rect.x, pos_rect.y, pos_rect.x + GRID_SIZE + 2, pos_rect.y + GRID_SIZE + 2);
			}
			break;

			case START_INDEX:
			{
				pos_rect.x = index.x * GRID_SIZE;
				pos_rect.y = index.y * GRID_SIZE;

				auto h_startPoint_brush = CreateSolidBrush(RGB(152, 243, 6));
				auto h_old_brush = SelectObject(hdc, h_startPoint_brush);
				Rectangle(hdc, pos_rect.x, pos_rect.y, pos_rect.x + GRID_SIZE + 2, pos_rect.y + GRID_SIZE + 2);
				SelectObject(hdc, h_old_brush);
				DeleteObject(h_startPoint_brush);
			}
			break;

			case END_INDEX:
			{
				pos_rect.x = index.x * GRID_SIZE;
				pos_rect.y = index.y * GRID_SIZE;

				auto h_endPoint_brush = CreateSolidBrush(RGB(255, 50, 0));
				auto h_old_brush = SelectObject(hdc, h_endPoint_brush);
				Rectangle(hdc, pos_rect.x, pos_rect.y, pos_rect.x + GRID_SIZE + 2, pos_rect.y + GRID_SIZE + 2);
				SelectObject(hdc, h_old_brush);
				DeleteObject(h_endPoint_brush);
			}
			break;

			case VISITED:
			{
				pos_rect.x = index.x * GRID_SIZE;
				pos_rect.y = index.y * GRID_SIZE;

				auto h_endPoint_brush = CreateSolidBrush(RGB(255, 255, 0));
				auto h_old_brush = SelectObject(hdc, h_endPoint_brush);
				Rectangle(hdc, pos_rect.x, pos_rect.y, pos_rect.x + GRID_SIZE + 2, pos_rect.y + GRID_SIZE + 2);
				SelectObject(hdc, h_old_brush);
				DeleteObject(h_endPoint_brush);
			}
			break;

			case SCHEDULED:
			{
				pos_rect.x = index.x * GRID_SIZE;
				pos_rect.y = index.y * GRID_SIZE;
				auto h_endPoint_brush = CreateSolidBrush(RGB(0, 0x80, 0xFF));
				auto h_old_brush = SelectObject(hdc, h_endPoint_brush);
				Rectangle(hdc, pos_rect.x, pos_rect.y, pos_rect.x + GRID_SIZE + 2, pos_rect.y + GRID_SIZE + 2);
				SelectObject(hdc, h_old_brush);
				DeleteObject(h_endPoint_brush);
			}
			break;

			default:
				break;
			}
		}
	}
	SelectObject(hdc, HOldBrush);
}

inline short Get_h(Pos pos) {
	return abs(index_end.x - pos.x) + abs(index_end.y - pos.y);
}

inline short Get_Distance(Pos pos1, Pos pos2) {
	return abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y);
}

list<Node*> open_list;
list<Node*> close_list;

bool Compare_node_p(const Node* a, const Node* b) {
	return a->f < b->f;
}

inline bool Check_Inside(Pos pos) {
	if (0 <= pos.x && pos.x < GRID_WIDTH) {
		if (0 <= pos.y && pos.y < GRID_HEIGHT) {
			return true;
		}
	}
	return false;
}

inline bool Check_Inside(int x, int y) {
	if (0 < x && x < GRID_WIDTH) {
		if (0 < y && y < GRID_HEIGHT) {
			return true;
		}
	}
	return false;
}

Node* Fine_open_list(Pos pos) {
	for (auto iter = open_list.begin(); iter != open_list.end(); iter++) {
		if ((*iter)->pos == pos) {
			return (*iter);
		}
	}
	return nullptr;
}

Node* Fine_close_list(Pos pos) {
	for (auto iter = close_list.begin(); iter != close_list.end(); iter++) {
		if ((*iter)->pos == pos) {
			return (*iter);
		}
	}
	return nullptr;
}

void Draw_Way() {
	Node* cur_node = &nodes[index_end.y][index_end.x];

	auto h_dc = GetDC(h_wnd);
	for (;;) {
		if (cur_node->pos == index_start) break;

		MoveToEx(h_dc, cur_node->pos.x * GRID_SIZE + (GRID_SIZE / 2), cur_node->pos.y * GRID_SIZE + (GRID_SIZE / 2), NULL);
		LineTo(h_dc, cur_node->parent->pos.x * GRID_SIZE + (GRID_SIZE / 2), cur_node->parent->pos.y * GRID_SIZE + (GRID_SIZE / 2));
		cur_node = cur_node->parent;
	}
	ReleaseDC(h_wnd, h_dc);
}

bool A_star() {
	nodes[index_start.y][index_start.x].Set(index_start, 0, Get_h(index_start), nullptr);
	open_list.emplace_back(&nodes[index_start.y][index_start.x]);

	for (;;) {
	// 노드 뽑기
	Node* visit_node = open_list.front();

	// 도착지 판단
	if (visit_node->pos == index_end) {
		return true;
	}

	// 노드 방문 처리 (오픈 리스트 제외, 클로즈 리스트 추가)
	if(visit_node->pos != index_start)
	tile_type[visit_node->pos.y][visit_node->pos.x] = VISITED;
	open_list.pop_front();
	close_list.emplace_back(visit_node);

	// 노드 생성, 오픈 리스트 추가
	Pos index;
	for (index.y = (visit_node->pos.y - 1); index.y <= (visit_node->pos.y + 1); index.y++) {
		for (index.x = (visit_node->pos.x - 1); index.x <= (visit_node->pos.x + 1); index.x++) {
			if (!Check_Inside(index))
				continue;
			if (tile_type[index.y][index.x] == OBSTACLE)
				continue;
			if (index.x == visit_node->pos.x && index.y == visit_node->pos.y)
				continue;
			if (nullptr != Fine_close_list(index))
				continue;

			auto neighbor_node = Fine_open_list(index);
			if (neighbor_node == nullptr) {
				// 오픈 리스트 추가
				if (1 < Get_Distance(visit_node->pos, index)) {
					nodes[index.y][index.x].Set(index, visit_node->g + 1.5, Get_h(index), visit_node);
				}
				else {
					nodes[index.y][index.x].Set(index, visit_node->g + 1, Get_h(index), visit_node);
				}
				open_list.emplace_back(&nodes[index.y][index.x]);
				if (index != index_end)
					tile_type[index.y][index.x] = SCHEDULED;
			}
			else {
				// 인접 노드 부모 재설정
				if (visit_node->g + 1 < neighbor_node->g) {
					neighbor_node->g = visit_node->g + 1;
					neighbor_node->parent = visit_node;
				}
			}
		}
	}

	// 렌더
	PAINTSTRUCT	ps;
	HDC	hdc;

	hdc = GetDC(h_wnd);
	RenderObstacle(hdc);
	RenderGrid(hdc);
	ReleaseDC(h_wnd, hdc);

	open_list.sort(Compare_node_p);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {


	switch (message)
	{

	case WM_KEYDOWN:
	{
		// ENTER 입력 시, wParam == 13
		if (wParam == 0x31) {
			A_star();
			Draw_Way();
		}
		else if (wParam == 0x32) {
			for (auto& t : tile_type) {
				for (auto& t2 : t) {
					if (t2 == START_INDEX || t2 == END_INDEX)
						continue;
					t2 = NONE;
				}
			}
			InvalidateRect(h_wnd, NULL, true);
		}
	}
	break;

	case WM_LBUTTONDOWN:
	{
		flag_drag = true;

		Pos pos_click;
		pos_click.x = GET_X_LPARAM(lParam);
		pos_click.y = GET_Y_LPARAM(lParam);

		Pos pos_clicked_tile;
		pos_clicked_tile.x = pos_click.x / GRID_SIZE;
		pos_clicked_tile.y = pos_click.y / GRID_SIZE;

		// 첫 선택 타일 속성에 따라 set draw type
		switch (tile_type[pos_clicked_tile.y][pos_clicked_tile.x])
		{
		case NONE:
			draw_type = DRAW_OBSTACLE;
			break;

		case OBSTACLE:
			draw_type = ERASE;
			break;

		case START_INDEX:
			draw_type = MOVE_START;
			break;

		case END_INDEX:
			draw_type = MOVE_END;
			break;

		default:
			break;
		}
	}
	break;

	case WM_LBUTTONUP:
	{
		flag_drag = false;
		break;
	}

	case WM_MOUSEMOVE:
	{
		Pos pos_mouse = { GET_X_LPARAM(lParam)  ,  GET_Y_LPARAM(lParam) };
		Pos index_prev_endIndex;

		if (flag_drag) {
			Pos index_tile;
			index_tile.x = pos_mouse.x / GRID_SIZE;
			index_tile.y = pos_mouse.y / GRID_SIZE;

			switch (draw_type)
			{
			case MOVE_START:
			{
				if (tile_type[index_tile.y][index_tile.x] == NONE) {
					tile_type[index_tile.y][index_tile.x] = START_INDEX;
					index_start = index_tile;

					tile_type[index_prev_start.y][index_prev_start.x] = NONE;
					index_prev_start = index_start;
				}
			}

			case MOVE_END:
			{
				if (tile_type[index_tile.y][index_tile.x] == NONE) {
					tile_type[index_tile.y][index_tile.x] = END_INDEX;
					index_end = index_tile;

					tile_type[index_prev_end.y][index_prev_end.x] = NONE;
					index_prev_end = index_end;
				}
			}
			break;

			case ERASE:
			{
				if (tile_type[index_tile.y][index_tile.x] == OBSTACLE) {
					tile_type[index_tile.y][index_tile.x] = NONE;
				}
			}
			break;

			case DRAW_OBSTACLE:
			{
				if (tile_type[index_tile.y][index_tile.x] == NONE) {
					tile_type[index_tile.y][index_tile.x] = OBSTACLE;
				}
			}
			break;

			default:
				break;
			}
			InvalidateRect(hWnd, NULL, true);
		}
	}
	break;

	case WM_CREATE:
	{
		g_hGridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
		g_hTileBrush = CreateSolidBrush(RGB(100, 100, 100));
	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT	ps;
		HDC	hdc;

		hdc = BeginPaint(hWnd, &ps);
		RenderObstacle(hdc);
		RenderGrid(hdc);
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_DESTROY:
	{
		DeleteObject(g_hTileBrush);
		DeleteObject(g_hGridPen);
		PostQuitMessage(0);
	}
	break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

int main() {
	Init_tile();
	Init_consol();
	Init_window();

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

BOOL Init_window() {
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

	h_wnd = CreateWindowW(
		TEXT(CLASS_NAME),
		TEXT(TITLE_NAME),
		WS_OVERLAPPEDWINDOW,
		CREATE_POS_X, CREATE_POS_Y, // 생성 위치
		WINDOW_WIDTH, WINDOW_HEIGHT, // 사이즈
		NULL,
		NULL,
		NULL,
		NULL);

	if (!h_wnd)
		return FALSE;

	ShowWindow(h_wnd, SW_SHOWNORMAL);
	UpdateWindow(h_wnd);
}

void RenderGrid(HDC hdc) {
	int pos_x = 0;
	int	pos_y = 0;
	HPEN hOldPen = (HPEN)SelectObject(hdc, g_hGridPen);

	// 세로선
	for (int i = 0; i <= GRID_WIDTH; i++) {
		MoveToEx(hdc, pos_x, 0, NULL);
		LineTo(hdc, pos_x, GRID_HEIGHT * GRID_SIZE);
		pos_x += GRID_SIZE;
	}
	// 가로선
	for (int i = 0; i <= GRID_HEIGHT; i++) {
		MoveToEx(hdc, 0, pos_y, NULL);
		LineTo(hdc, GRID_WIDTH * GRID_SIZE, pos_y);
		pos_y += GRID_SIZE;
	}
	SelectObject(hdc, hOldPen);
}