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
	OBSTACLE,	// Ž�� �Ұ� Ÿ��
	START,// ������ Ÿ��
	END,	// ������ Ÿ��
	VISITED,	// Ŭ���� ����Ʈ�� ���� Ÿ��
	SCHEDULED,	// ���� ����Ʈ�� ���� Ÿ��
	SEARCH,		// ��带 ��������� Ž���� Ÿ��
};

enum class eSEARCH_RESULT : unsigned char {
	NONE,
	UNABLE,
	CORNER,
};

// �̳� Ŭ����?, �����ڿ��� �����Ҵ�?
struct Node {
	Pos pos;
	eDIR dir;

	float g; // ����������� �̵��Ÿ� (���� �θ���� �Ÿ�)
	float h; // ���������� �Ÿ�
	float f; // g + h, f�� ���� ���� ��带 ã���� ����������.
	Node* parent = nullptr;

	stRGB rgb;
public:
	void Set(Pos  pos, float g, float h, Node* const parent, eDIR dir);
};

// �����ڷ� Ÿ�� �������� ����
#define GRID_WIDTH 100
#define GRID_HEIGHT 50

// ������ ����ü
struct GHF {
	float g;
	float h;
	float f;

	bool flag_node = false;
};

extern GHF ghf[GRID_HEIGHT][GRID_WIDTH];

extern eTILE_STATE tile_type[GRID_HEIGHT][GRID_WIDTH]; // �����ڿ��� ���޹ް�?
extern stRGB tile_color[GRID_HEIGHT][GRID_WIDTH]; // winapi ������ ����
extern Node nodes[GRID_HEIGHT][GRID_WIDTH]; // �����ڿ��� ���� �Ҵ�?
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
// Ŀ�� üũ
////////////////////////////////////////////////

// ����, ����
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

// �밢��
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
// ���� ���� ���� Ž��
////////////////////////////////////////////////

// ����, ����
bool Search_Curve_UU(Pos node_pos, Pos* p_curve_pos);
bool Search_Curve_DD(Pos node_pos, Pos* p_curve_pos);
bool Search_Curve_LL(Pos node_pos, Pos* p_curve_pos);
bool Search_Curve_RR(Pos node_pos, Pos* p_curve_pos);

// �밢��
bool Search_Curve_UL(Pos node_pos, Pos* p_curve_pos);
bool Search_Curve_UR(Pos node_pos, Pos* p_curve_pos);
bool Search_Curve_DL(Pos node_pos, Pos* p_curve_pos);
bool Search_Curve_DR(Pos node_pos, Pos* p_curve_pos);

// for JPS
// ���¸���Ʈ�� ��� �ֱ� (��� ����)
bool PushBack_OpenList(Node* const parent_node, Pos node_pos, eDIR node_dir);
// *** parent_node DIR �������� Ž�� �� ������ (ex. UU �ִ� 3����, UR �ִ� 5����)
void OpenList_PushBack_Proc(Node* const parent_node);
// JPS
bool JPS(Pos start, Pos end);

// ����� ��
void Mark_SEARCH(Pos tile_pos);