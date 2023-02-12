#include "stdafx.h"
#include "WndProc.h"
#include "JPS.h"
#include "enums.h"
#include "stRGB.h"

using namespace std;

#define WHEEL_UP	120
#define WHEEL_DOWN	-120
#define TEXT_HEIGHT 1.03

HPEN h_grid_pen;
HBRUSH h_obstacle_brush;
HBRUSH h_start_tile_brush;
HBRUSH h_end_tile_brush;
HBRUSH h_visit_tile_brush;
HBRUSH h_schedule_tile_brush;

unsigned char GRID_SIZE = 60;

unsigned char move_height = 0;
unsigned char move_width = 0;

enum class eDRAW_TYPE {
	DRAW_OBSTACLE,
	ERASE_OBSTACLE,
	MOVE_START,
	MOVE_END,
};

eDRAW_TYPE draw_type;

HWND h_wnd;
MSG msg;

bool flag_drag = false;

void RenderGrid(HDC hdc) {
	int pos_x = 0;
	int	pos_y = 0;
	HPEN hOldPen = (HPEN)SelectObject(hdc, h_grid_pen);

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

wchar_t Get_Dir_char(eDIR dir) {
	switch (dir)
	{
	case eDIR::DIR_UU:
		return L'↑';
	case eDIR::DIR_UL:
		return L'↖';
	case eDIR::DIR_UR:
		return L'↗';
	case eDIR::DIR_LL:
		return L'←';
	case eDIR::DIR_RR:
		return L'→';
	case eDIR::DIR_DD:
		return L'↓';
	case eDIR::DIR_DL:
		return L'↙';
	case eDIR::DIR_DR:
		return L'↘';
	case eDIR::DIR_ALL:
		return L'▶';

	case eDIR::NONE:
	default:
		throw;
		break;
	}
}

bool is_showInfo = true;

void Show_NodeInfo(Pos node_pos ,Pos show_pos) {
	if (is_showInfo) {
		PAINTSTRUCT	ps;
		HDC	hdc;
		hdc = GetDC(h_wnd);

		if (Fine_open_list(node_pos) || Fine_close_list(node_pos)) {
			SetBkMode(hdc, TRANSPARENT);
			auto const& node = nodes[node_pos.second][node_pos.first];
			wchar_t wchar_buf[128];

			//// Show Dir
			//wchar_buf[0] = Get_Dir_char(node.dir);
			//TextOut(hdc, show_pos.first, show_pos.second, &wchar_buf[0], 1);

			// Show x, y
			swprintf_s(wchar_buf, sizeof(wchar_buf), L" %c (%d, %d)", Get_Dir_char(node.dir), node_pos.first, node_pos.second);
			TextOutW(hdc, show_pos.first - (move_width * GRID_SIZE), show_pos.second - (move_height * GRID_SIZE), wchar_buf, wcslen(wchar_buf));

			//// Show g, h, f
			//if (ghf[node_pos.second][node_pos.first].flag_node == true) {
			//	swprintf_s(wchar_buf, sizeof(wchar_buf), L" G : %.2f", ghf[node_pos.second][node_pos.first].g);
			//	TextOutW(hdc, show_pos.first - (move_width * GRID_SIZE), show_pos.second - (move_height * GRID_SIZE) + 15, wchar_buf, wcslen(wchar_buf));

			//	swprintf_s(wchar_buf, sizeof(wchar_buf), L" H : %.2f", ghf[node_pos.second][node_pos.first].h);
			//	TextOutW(hdc, show_pos.first - (move_width * GRID_SIZE), show_pos.second - (move_height * GRID_SIZE) + 30, wchar_buf, wcslen(wchar_buf));

			//	swprintf_s(wchar_buf, sizeof(wchar_buf), L" F : %.2f", ghf[node_pos.second][node_pos.first].f);
			//	TextOutW(hdc, show_pos.first - (move_width * GRID_SIZE), show_pos.second - (move_height * GRID_SIZE) + 45, wchar_buf, wcslen(wchar_buf));
			//}

		}

		ReleaseDC(h_wnd, hdc);
	}
}

void RenderTile(HDC	hdc) {
	SelectObject(hdc, GetStockObject(NULL_PEN));

	//if (30 <= GRID_SIZE)
	//	is_showInfo = true;
	//else
	//	is_showInfo = false;

	Pos rect_pos{ 0,0 };
	Pos node_pos{ 0, 0 };
	for (node_pos.first = 0; node_pos.first < GRID_WIDTH; node_pos.first++) {
		for (node_pos.second = 0; node_pos.second < GRID_HEIGHT; node_pos.second++) {
			switch (tile_type[node_pos.second][node_pos.first])			{
				case eTILE_STATE::OBSTACLE:{
					rect_pos.first = node_pos.first * GRID_SIZE;
					rect_pos.second = node_pos.second * GRID_SIZE;

					HBRUSH h_old_brush = (HBRUSH)SelectObject(hdc, h_obstacle_brush);
					Rectangle(hdc, rect_pos.first - (move_width * GRID_SIZE), rect_pos.second - (move_height * GRID_SIZE), rect_pos.first - (move_width * GRID_SIZE) + GRID_SIZE + 2, rect_pos.second - (move_height * GRID_SIZE) + GRID_SIZE + 2);
					SelectObject(hdc, h_old_brush);
				}
				break;

				case eTILE_STATE::START:{
					rect_pos.first = node_pos.first * GRID_SIZE;
					rect_pos.second = node_pos.second * GRID_SIZE;

					HBRUSH h_old_brush = (HBRUSH)SelectObject(hdc, h_start_tile_brush);
					Rectangle(hdc, rect_pos.first - (move_width * GRID_SIZE), rect_pos.second - (move_height * GRID_SIZE), rect_pos.first - (move_width * GRID_SIZE) + GRID_SIZE + 2, rect_pos.second - (move_height * GRID_SIZE) + GRID_SIZE + 2);
					SelectObject(hdc, h_old_brush);

					Show_NodeInfo(node_pos, rect_pos);
				}
				break;

				case eTILE_STATE::END:{
					rect_pos.first = node_pos.first * GRID_SIZE;
					rect_pos.second = node_pos.second * GRID_SIZE;

					HBRUSH h_old_brush = (HBRUSH)SelectObject(hdc, h_end_tile_brush);
					Rectangle(hdc, rect_pos.first - (move_width * GRID_SIZE), rect_pos.second - (move_height * GRID_SIZE), rect_pos.first - (move_width * GRID_SIZE) + GRID_SIZE + 2, rect_pos.second - (move_height * GRID_SIZE) + GRID_SIZE + 2);
					SelectObject(hdc, h_old_brush);

					Show_NodeInfo(node_pos, rect_pos);
				}
				break;

				case eTILE_STATE::VISITED:{
					rect_pos.first = node_pos.first * GRID_SIZE;
					rect_pos.second = node_pos.second * GRID_SIZE;

					HBRUSH h_old_brush = (HBRUSH)SelectObject(hdc, h_visit_tile_brush);
					Rectangle(hdc, rect_pos.first - (move_width * GRID_SIZE), rect_pos.second - (move_height * GRID_SIZE), rect_pos.first - (move_width * GRID_SIZE) + GRID_SIZE + 2, rect_pos.second - (move_height * GRID_SIZE) + GRID_SIZE + 2);
					SelectObject(hdc, h_old_brush);

					Show_NodeInfo(node_pos, rect_pos);
				}
				break;

				case eTILE_STATE::SCHEDULED:{
					rect_pos.first = node_pos.first * GRID_SIZE;
					rect_pos.second = node_pos.second * GRID_SIZE;

					HBRUSH h_old_brush = (HBRUSH)SelectObject(hdc, h_schedule_tile_brush);
					Rectangle(hdc, rect_pos.first - (move_width * GRID_SIZE), rect_pos.second - (move_height * GRID_SIZE), rect_pos.first - (move_width * GRID_SIZE) + GRID_SIZE + 2, rect_pos.second - (move_height * GRID_SIZE) + GRID_SIZE + 2);
					SelectObject(hdc, h_old_brush);

					Show_NodeInfo(node_pos, rect_pos);
				}
				break;

				//case eTILE_STATE::SEARCH:{
				//	rect_pos.first = node_pos.first * GRID_SIZE;
				//	rect_pos.second = node_pos.second * GRID_SIZE;

				//	HBRUSH h_search_brush = CreateSolidBrush(RGB(tile_color[node_pos.second][node_pos.first].R, tile_color[node_pos.second][node_pos.first].G, tile_color[node_pos.second][node_pos.first].B));
				//	auto h_old_brush = SelectObject(hdc, h_search_brush);
				//	Rectangle(hdc, rect_pos.first, rect_pos.second, rect_pos.first + GRID_SIZE + 2, rect_pos.second + GRID_SIZE + 2);
				//	SelectObject(hdc, h_old_brush);
				//	DeleteObject(h_search_brush);

				//	Show_NodeInfo(node_pos, rect_pos);
				//}
				//break;

				default:
					break;
			}
		}
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch (message) {
	case WM_MOUSEWHEEL:{


		switch ((short)HIWORD(wParam)) {
			case WHEEL_UP: {
				if (GRID_SIZE < 0xFF) {
					GRID_SIZE++;
				}
				break;
			}

			case WHEEL_DOWN: {
				if (0 < GRID_SIZE) {
					GRID_SIZE--;
				}
				break;
			}
		}

		printf("GRID_SIZE : %d \n", GRID_SIZE);
		InvalidateRect(h_wnd, NULL, true);
		break;
	}

	case WM_KEYDOWN: {
		// 화살표 입력
		if (37 <= wParam && wParam <= 40) {
			// 좌
			if (wParam == 37) {
				if (0 < move_width)
					move_width--;
			}
			// 우
			else if (wParam == 39) {
				if (move_width < UCHAR_MAX)
					move_width++;
			}
			// 위
			else if (wParam == 38) {
				if (0 < move_height)
					move_height--;
			}
			// 아래
			else if (wParam == 40) {
				if (move_height < UCHAR_MAX)
					move_height++;
			}
			InvalidateRect(h_wnd, NULL, true);
			break;
		}

		switch (wParam) {
		case 0x31: {
			bool is_way = JPS(start_point, end_point);
			if (is_way) {
				Draw_Way();
			}
			break;
		}

				 // 타일 정리
		case 0x32: {
			move_height = 0;
			move_width = 0;

			ZeroMemory(tile_type, GRID_WIDTH * GRID_HEIGHT);
			open_list.clear();
			close_list.clear();

			start_point = Initial_start_point;
			end_point = Initial_end_point;
			tile_type[start_point.second][start_point.first] = eTILE_STATE::START;
			tile_type[end_point.second][end_point.first] = eTILE_STATE::END;

			InvalidateRect(h_wnd, NULL, true);
			break;
		}

		default:
			break;
		}

		break;
	}

	case WM_LBUTTONDOWN: {
		flag_drag = true;
		Pos tile_pos{ GET_X_LPARAM(lParam) / GRID_SIZE , GET_Y_LPARAM(lParam) / GRID_SIZE };

		switch (tile_type[tile_pos.second][tile_pos.first])
		{
		case eTILE_STATE::NONE:
			draw_type = eDRAW_TYPE::DRAW_OBSTACLE;
			break;

		case eTILE_STATE::OBSTACLE:
			draw_type = eDRAW_TYPE::ERASE_OBSTACLE;
			break;

		case eTILE_STATE::START:
			draw_type = eDRAW_TYPE::MOVE_START;
			break;

		case eTILE_STATE::END:
			draw_type = eDRAW_TYPE::MOVE_END;
			break;

		default:
			break;
		}

		break;
	}

	case WM_LBUTTONUP: {
		flag_drag = false;
		break;
	}

	case WM_MOUSEMOVE: {
		if (flag_drag) {
			Pos mouse_tile_pos{ GET_X_LPARAM(lParam) / GRID_SIZE , GET_Y_LPARAM(lParam) / GRID_SIZE };

			switch (draw_type) {
			case eDRAW_TYPE::DRAW_OBSTACLE: {
				if (tile_type[mouse_tile_pos.second][mouse_tile_pos.first] == eTILE_STATE::NONE) {
					tile_type[mouse_tile_pos.second][mouse_tile_pos.first] = eTILE_STATE::OBSTACLE;
				}
				break;
			}

			case eDRAW_TYPE::ERASE_OBSTACLE: {
				if (tile_type[mouse_tile_pos.second][mouse_tile_pos.first] == eTILE_STATE::OBSTACLE) {
					tile_type[mouse_tile_pos.second][mouse_tile_pos.first] = eTILE_STATE::NONE;
				}
				break;
			}

			case eDRAW_TYPE::MOVE_START: {
				if (tile_type[mouse_tile_pos.second][mouse_tile_pos.first] == eTILE_STATE::NONE) {
					tile_type[start_point.second][start_point.first] = eTILE_STATE::NONE;
					tile_type[mouse_tile_pos.second][mouse_tile_pos.first] = eTILE_STATE::START;
					start_point = mouse_tile_pos;
				}
				break;
			}

			case eDRAW_TYPE::MOVE_END: {
				if (tile_type[mouse_tile_pos.second][mouse_tile_pos.first] == eTILE_STATE::NONE) {
					tile_type[end_point.second][end_point.first] = eTILE_STATE::NONE;
					tile_type[mouse_tile_pos.second][mouse_tile_pos.first] = eTILE_STATE::END;
					end_point = mouse_tile_pos;
				}
				break;
			}

			default:
				break;
			}
			InvalidateRect(hWnd, NULL, true);
		}

		break;
	}

	case WM_CREATE: {
		h_grid_pen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
		h_obstacle_brush = CreateSolidBrush(RGB(100, 100, 100));
		h_start_tile_brush = CreateSolidBrush(RGB(152, 243, 6));
		h_end_tile_brush = CreateSolidBrush(RGB(255, 50, 0));
		h_visit_tile_brush = CreateSolidBrush(RGB(255, 255, 0));
		h_schedule_tile_brush = CreateSolidBrush(RGB(0, 0x80, 0xFF));

		tile_type[start_point.second][start_point.first] = eTILE_STATE::START;
		tile_type[end_point.second][end_point.first] = eTILE_STATE::END;
		break;
	}

	case WM_PAINT: {
		PAINTSTRUCT	ps;
		HDC	hdc;
		hdc = BeginPaint(h_wnd, &ps);

		WM_PAINT_Proc();

		EndPaint(h_wnd, &ps);
		break;
	}

	case WM_DESTROY: {
		DeleteObject(h_obstacle_brush);
		DeleteObject(h_grid_pen);
		PostQuitMessage(0);
		break;
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void Draw_Way() {
	Node* cur_node = &nodes[end_point.second][end_point.first];

	auto h_dc = GetDC(h_wnd);
	for (;;) {
		if (cur_node->pos == start_point) break;

		auto pen = CreatePen(PS_SOLID, 3, RGB(255, 0, 30));
		auto h_old_pen = SelectObject(h_dc, pen);

		MoveToEx(h_dc,	cur_node->pos.first * GRID_SIZE + (GRID_SIZE / 2) - (move_width * GRID_SIZE),			cur_node->pos.second * GRID_SIZE + (GRID_SIZE / 2) - (move_height * GRID_SIZE),			NULL);
		LineTo(h_dc,	cur_node->parent->pos.first * GRID_SIZE + (GRID_SIZE / 2) - (move_width * GRID_SIZE),	cur_node->parent->pos.second * GRID_SIZE + (GRID_SIZE / 2) - (move_height * GRID_SIZE));
		cur_node = cur_node->parent;

		SelectObject(h_dc, h_old_pen);
		DeleteObject(pen);
	}
	ReleaseDC(h_wnd, h_dc);
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
		TEXT(CLASS_NAME), // lpClassName
		TEXT(TITLE_NAME), //lpWindowName
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL, // window style
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

void WM_PAINT_Proc() {
	PAINTSTRUCT	ps;
	HDC	hdc;

	hdc = GetDC(h_wnd);
	RenderTile(hdc);
	RenderGrid(hdc);
	ReleaseDC(h_wnd, hdc);
}

void Window_Clear() {
	PAINTSTRUCT	ps;
	HDC	hdc;
	hdc = GetDC(h_wnd);
	SelectObject(hdc, GetStockObject(WHITE_BRUSH));
	Rectangle(hdc, 0, 0, 2000, 2000);
	ReleaseDC(h_wnd, hdc);
}