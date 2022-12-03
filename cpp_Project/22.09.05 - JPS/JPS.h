#pragma once
#include <list>
#include <utility>
#include "WndProc.h"
#include "stRGB.h"

typedef std::pair<int, int> Pos;

enum class eDIR : unsigned char {
	NONE,
	DIR_UU,
	DIR_UL,
	DIR_UR,
	DIR_LL,
	DIR_RR,
	DIR_DD,
	DIR_DL,
	DIR_DR,
	DIR_ALL,
};

enum class eTILE_STATE : unsigned char {
	NONE,
	OBSTACLE,	// 탐색 불가 타일
	START,// 시작점 타일
	END,	// 도착점 타일
	VISITED,	// 클로즈 리스트에 속한 타일
	SCHEDULED,	// 오픈 리스트에 속한 타일
	SEARCH,		// 노드를 만들기위해 탐색한 타일
};

enum class eSEARCH_RESULT : unsigned char {
	NONE,
	UNABLE,
	CORNER,
};

// 이너 클래스?, 생성자에서 동적할당?
struct Node {
	Pos pos;
	eDIR dir;

	float g; // 출발점에서의 이동거리 (직전 부모와의 거리)
	float h; // 목적지와의 거리
	float f; // g + h, f가 가장 작은 노드를 찾으며 퍼저나간다.
	Node* parent = nullptr;

	stRGB rgb;
public:
	void Set(Pos  pos, float g, float h, Node* const parent, eDIR dir);
};

// 생성자로 타일 스케일을 받음
#define GRID_WIDTH 100
#define GRID_HEIGHT 50

// 디버깅용 구조체
struct GHF {
	float g;
	float h;
	float f;

	bool flag_node = false;
};

extern GHF ghf[GRID_HEIGHT][GRID_WIDTH];

extern eTILE_STATE tile_type[GRID_HEIGHT][GRID_WIDTH]; // 생성자에서 전달받게?
extern stRGB tile_color[GRID_HEIGHT][GRID_WIDTH]; // winapi 디버깅용 변수
extern Node nodes[GRID_HEIGHT][GRID_WIDTH]; // 생성자에서 동적 할당?
extern std::list<Node*> open_list;
extern std::list<Node*> close_list;

extern Pos Initial_start_point;
extern Pos Initial_end_point;

extern Pos start_point;
extern Pos end_point;

inline float Get_Distance(Pos pos1, Pos pos2) {
	return abs(pos1.first - pos2.first) + abs(pos1.second - pos2.second);
}

inline float Get_h(Pos pos) {
	return Get_Distance(end_point, pos);
}

inline bool Check_Over_X(int index_x) {
	return  index_x <= (GRID_WIDTH - 1);
}

inline bool Check_Under_X(int index_x) {
	return 0 <= index_x;
}

inline bool Check_X(int index_x) {
	return Check_Over_X(index_x) && Check_Under_X(index_x);
}

inline bool Check_Over_Y(int index_y) {
	return  index_y <= (GRID_HEIGHT - 1);
}

inline bool Check_Under_Y(int index_y) {
	return 0 <= index_y;
}

inline bool Check_Y(int index_y) {
	return Check_Over_Y(index_y) && Check_Under_Y(index_y);
}

inline bool Check_Obstacle(Pos check_point) {
	return tile_type[check_point.second][check_point.first] == eTILE_STATE::OBSTACLE;
}

bool Fine_open_list(Pos pos, Node** const p = nullptr);
bool Fine_close_list(Pos pos, Node** const p = nullptr);

bool Compare_node_p(Node* a, Node* b);

////////////////////////////////////////////////
// 커브 체크
////////////////////////////////////////////////

// 수직, 수평
bool Check_Curve_UU_L(const Pos check_point);
bool Check_Curve_UU_R(const Pos check_point);
eSEARCH_RESULT Check_Curve_UU(const Pos check_point);
bool Check_Curve_DD_L(const Pos check_point);
bool Check_Curve_DD_R(const Pos check_point);
eSEARCH_RESULT Check_Curve_DD(const Pos check_point);
bool Check_Curve_RR_L(const Pos check_point);
bool Check_Curve_RR_R(const Pos check_point);
eSEARCH_RESULT Check_Curve_RR(const Pos check_point);
bool Check_Curve_LL_L(const Pos check_point);
bool Check_Curve_LL_R(const Pos check_point);
eSEARCH_RESULT Check_Curve_LL(const Pos check_point);

// 대각선
bool Check_Curve_UR_L(const Pos check_point);
bool Check_Curve_UR_R(const Pos check_point);
eSEARCH_RESULT Check_Curve_UR(const Pos check_point);
bool Check_Curve_DR_L(const Pos check_point);
bool Check_Curve_DR_R(const Pos check_point);
eSEARCH_RESULT Check_Curve_DR(const Pos check_point);
bool Check_Curve_DL_L(const Pos check_point);
bool Check_Curve_DL_R(const Pos check_point);
eSEARCH_RESULT Check_Curve_DL(const Pos check_point);
bool Check_Curve_UL_L(const Pos check_point);
bool Check_Curve_UL_R(const Pos check_point);
eSEARCH_RESULT Check_Curve_UL(Pos check_point);

////////////////////////////////////////////////
// 방향 기준 전부 탐색
////////////////////////////////////////////////

// 수평, 수직
bool Search_Curve_UU(Pos node_pos, Pos* p_curve_pos);
bool Search_Curve_DD(Pos node_pos, Pos* p_curve_pos);
bool Search_Curve_LL(Pos node_pos, Pos* p_curve_pos);
bool Search_Curve_RR(Pos node_pos, Pos* p_curve_pos);

// 대각선
bool Search_Curve_UL(Pos node_pos, Pos* p_curve_pos);
bool Search_Curve_UR(Pos node_pos, Pos* p_curve_pos);
bool Search_Curve_DL(Pos node_pos, Pos* p_curve_pos);
bool Search_Curve_DR(Pos node_pos, Pos* p_curve_pos);

// for JPS
// 오픈리스트에 노드 넣기 (노드 생성)
bool PushBack_OpenList(Node* const parent_node, Pos node_pos, eDIR node_dir);
// *** parent_node DIR 기준으로 탐색 및 노드생성 (ex. UU 최대 3방향, UR 최대 5방향)
void OpenList_PushBack_Proc(Node* const parent_node);
// JPS
bool JPS(Pos start, Pos end);

// 디버깅 용
void Mark_SEARCH(Pos tile_pos);