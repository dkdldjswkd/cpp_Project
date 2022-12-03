#include "stdafx.h"
#include "JPS.h"

GHF ghf[GRID_HEIGHT][GRID_WIDTH];

eTILE_STATE tile_type[GRID_HEIGHT][GRID_WIDTH];
stRGB tile_color[GRID_HEIGHT][GRID_WIDTH];
Node nodes[GRID_HEIGHT][GRID_WIDTH]; // 생성자에서 동적 할당?

std::list<Node*> open_list;
std::list<Node*> close_list;

// 출발지, 도착지 디폴트 값
Pos Initial_start_point{ 7,3 };
Pos Initial_end_point{ 15, 7 };

Pos start_point{ Initial_start_point };
Pos end_point{ Initial_end_point };

stRGB search_rgb;

void Node::Set(Pos pos, float g, float h, Node* const parent, eDIR dir) {
	this->pos = pos;
	this->g = g;
	this->h = h;
	f = g + h;
	this->parent = parent;
	this->dir = dir;
}

bool Fine_open_list(Pos pos, Node** const p) {
	for (auto iter = open_list.begin(); iter != open_list.end(); iter++) {
		if ((*iter)->pos == pos) {
			if (p != nullptr)
				*p = *iter;
			return true;
		}
	}
	return false;
}

bool Fine_close_list(Pos pos, Node** const p) {
	for (auto iter = close_list.begin(); iter != close_list.end(); iter++) {
		if ((*iter)->pos == pos) {
			if(p != nullptr)
				*p = *iter;
			return true;
		}
	}
	return false;
}

bool Compare_node_p(Node* a, Node* b) {
	return a->f < b->f;
}

// 0  
// x ↑
bool Check_Curve_UU_L(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 왼쪽위칸 뚫렸다면,
	if (Check_Under_X(check_point.first - 1) && Check_Under_Y(check_point.second - 1)) {
		if (tile_type[check_point.second - 1][check_point.first - 1] != eTILE_STATE::OBSTACLE) {
			// 왼쪽 장애물 판단
			if (tile_type[check_point.second][check_point.first - 1] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}
	}

	return false;
}

//    0
// ↑ x
bool Check_Curve_UU_R(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 오른쪽위칸 뚫렸다면,
	if (Check_Over_X(check_point.first + 1) && Check_Under_Y(check_point.second - 1)) {
		if (tile_type[check_point.second - 1][check_point.first + 1] != eTILE_STATE::OBSTACLE) {
			// 오른쪽 장애물 판단
			if (tile_type[check_point.second][check_point.first + 1] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}
	}

	return false;
}

// 0  0
// x↑x
eSEARCH_RESULT Check_Curve_UU(const Pos check_point) {
	if (Check_Obstacle(check_point)) 
		return eSEARCH_RESULT::UNABLE;
	if (!Check_Under_Y(check_point.second))
		return eSEARCH_RESULT::UNABLE;

	if (Check_Curve_UU_L(check_point))
		return  eSEARCH_RESULT::CORNER;
	if (Check_Curve_UU_R(check_point))
		return  eSEARCH_RESULT::CORNER;

	return  eSEARCH_RESULT::NONE;
}

//↓ x
//   0
bool Check_Curve_DD_L(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 오른쪽아래칸 뚫렸다면,
	if (Check_Over_X(check_point.first + 1) && Check_Over_Y(check_point.second + 1)) {
		if (tile_type[check_point.second + 1][check_point.first + 1] != eTILE_STATE::OBSTACLE) {
			// 오론쪽 장애물 판단
			if (tile_type[check_point.second][check_point.first + 1] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}
	}

	return false;
}

// x ↓
// 0   
bool Check_Curve_DD_R(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 왼쪽아래칸 뚫렸다면,
	if (Check_Under_X(check_point.first - 1) && Check_Over_Y(check_point.second + 1)) {
		if (tile_type[check_point.second + 1][check_point.first - 1] != eTILE_STATE::OBSTACLE) {
			// 왼쪽 장애물 판단
			if (tile_type[check_point.second][check_point.first - 1] == eTILE_STATE::OBSTACLE) {
				// 왼쪽아래칸 코너
				return true;
			}
		}
	}

	return false;
}

// x ↓ x
// 0    0
eSEARCH_RESULT Check_Curve_DD(const Pos check_point) {
	if (Check_Obstacle(check_point))
		return eSEARCH_RESULT::UNABLE;
	if (!Check_Over_Y(check_point.second))
		return eSEARCH_RESULT::UNABLE;

	if (Check_Curve_DD_L(check_point))
		return  eSEARCH_RESULT::CORNER;
	if (Check_Curve_DD_R(check_point))
		return  eSEARCH_RESULT::CORNER;

	return eSEARCH_RESULT::NONE;
}

// x  0
// →  
bool Check_Curve_RR_L(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 오른쪽위칸 뚫렸다면,
	if (Check_Under_Y(check_point.second - 1) && Check_Over_X(check_point.first + 1)) {
		if (tile_type[check_point.second - 1][check_point.first + 1] != eTILE_STATE::OBSTACLE) {
			// 위쪽 장애물 판단
			if (tile_type[check_point.second - 1][check_point.first] == eTILE_STATE::OBSTACLE) {
				// 오른쪽위칸 코너
				return true;
			}
		}
	}

	return false;
}

// →  
// x  0
bool Check_Curve_RR_R(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 오른쪽아래칸 뚫렸다면,
	if (Check_Over_Y(check_point.second + 1) && Check_Over_X(check_point.first + 1)) {
		if (tile_type[check_point.second + 1][check_point.first + 1] != eTILE_STATE::OBSTACLE) {
			// 아래쪽 장애물 판단
			if (tile_type[check_point.second + 1][check_point.first] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}
	}

	return false;
}

// x  0
// →  
// x  0
eSEARCH_RESULT Check_Curve_RR(const Pos check_point ) {
	if (Check_Obstacle(check_point))
		return eSEARCH_RESULT::UNABLE;
	if (!Check_Over_X(check_point.first))
		return eSEARCH_RESULT::UNABLE;

	if (Check_Curve_RR_L(check_point))
		return  eSEARCH_RESULT::CORNER;
	if (Check_Curve_RR_R(check_point))
		return  eSEARCH_RESULT::CORNER;

	return eSEARCH_RESULT::NONE;
}

//    ←
// 0  x
bool Check_Curve_LL_L(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 왼쪽아래칸 뚫렸다면,
	if (Check_Over_Y(check_point.second + 1) && Check_Under_X(check_point.first - 1)) {
		if (tile_type[check_point.second + 1][check_point.first - 1] != eTILE_STATE::OBSTACLE) {
			// 아래쪽 장애물 판단
			if (tile_type[check_point.second + 1][check_point.first] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}
	}

	return false;
}

// 0  x
//    ←
bool Check_Curve_LL_R(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 왼쪽위칸 뚫렸다면,
	if (Check_Under_Y(check_point.second - 1) && Check_Under_X(check_point.first - 1)) {
		if (tile_type[check_point.second - 1][check_point.first - 1] != eTILE_STATE::OBSTACLE) {
			// 위쪽 장애물 판단
			if (tile_type[check_point.second - 1][check_point.first] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}
	}

	return false;
}

// 0  x
//    ←
// 0  x
eSEARCH_RESULT Check_Curve_LL(const Pos check_point ) {
	if (Check_Obstacle(check_point))
		return eSEARCH_RESULT::UNABLE;
	if (!Check_Under_X(check_point.first))
		return eSEARCH_RESULT::UNABLE;

	if (Check_Curve_LL_L(check_point))
		return  eSEARCH_RESULT::CORNER;
	if (Check_Curve_LL_R(check_point))
		return  eSEARCH_RESULT::CORNER;

	return eSEARCH_RESULT::NONE;
}

// 0  
// x ↗
bool Check_Curve_UR_L(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 왼쪽위칸 뚫렸다면,
	if (Check_Under_X(check_point.first - 1) && Check_Under_Y(check_point.second - 1)) {
		if (tile_type[check_point.second - 1][check_point.first - 1] != eTILE_STATE::OBSTACLE) {
			// 왼쪽 장애물 판단
			if (tile_type[check_point.second][check_point.first - 1] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}
	}

	return false;
}

// ↗
// x  0  
bool Check_Curve_UR_R(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 오른쪽아래칸 뚫렸다면,
	if (Check_Over_Y(check_point.second + 1) && Check_Over_X(check_point.first + 1)) {
		if (tile_type[check_point.second + 1][check_point.first + 1] != eTILE_STATE::OBSTACLE) {
			// 아래쪽 장애물 판단
			if (tile_type[check_point.second + 1][check_point.first] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}
	}

	return false;
}

// 0  
// x ↗
//   x  0  
eSEARCH_RESULT Check_Curve_UR(const Pos check_point ) {
	if (Check_Obstacle(check_point))
		return eSEARCH_RESULT::UNABLE;
	if (!Check_Over_X(check_point.first))
		return eSEARCH_RESULT::UNABLE;
	if (!Check_Under_Y(check_point.second))
		return eSEARCH_RESULT::UNABLE;

	if (Check_Curve_UR_L(check_point))
		return  eSEARCH_RESULT::CORNER;
	if (Check_Curve_UR_R(check_point))
		return  eSEARCH_RESULT::CORNER;

	return eSEARCH_RESULT::NONE;
}

// x  0
// ↘
bool Check_Curve_DR_L(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 오른쪽위칸 뚫렸다면,
	if (Check_Under_Y(check_point.second - 1) && Check_Over_X(check_point.first + 1)) {
		if (tile_type[check_point.second - 1][check_point.first + 1] != eTILE_STATE::OBSTACLE) {
			// 위쪽 장애물 판단
			if (tile_type[check_point.second - 1][check_point.first] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}
	}

	return false;
}

// x ↘
// 0 
bool Check_Curve_DR_R(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 왼쪽아래칸 뚫렸다면,
	if (Check_Under_X(check_point.first - 1) && Check_Over_Y(check_point.second + 1)) {
		if (tile_type[check_point.second + 1][check_point.first - 1] != eTILE_STATE::OBSTACLE) {
			// 왼쪽 장애물 판단
			if (tile_type[check_point.second][check_point.first - 1] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}
	}

	return false;
}

//   x  0  
// x ↘
// 0 
eSEARCH_RESULT Check_Curve_DR(const Pos check_point) {
	if (Check_Obstacle(check_point))
		return eSEARCH_RESULT::UNABLE;
	if (!Check_Over_X(check_point.first))
		return eSEARCH_RESULT::UNABLE;
	if (!Check_Over_Y(check_point.second))
		return eSEARCH_RESULT::UNABLE;

	if (Check_Curve_DR_L(check_point))
		return  eSEARCH_RESULT::CORNER;
	if (Check_Curve_DR_R(check_point))
		return  eSEARCH_RESULT::CORNER;

	return eSEARCH_RESULT::NONE;
}

// ↙ x
//    0
bool Check_Curve_DL_L(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 오른쪽아래칸 뚫렸다면,
	if (Check_Over_X(check_point.first + 1) && Check_Over_Y(check_point.second + 1)) {
		if (tile_type[check_point.second + 1][check_point.first + 1] != eTILE_STATE::OBSTACLE) {
			// 오론쪽 장애물 판단
			if (tile_type[check_point.second][check_point.first + 1] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}

	}

	return false;
}

// 0 x 
//   ↙
bool Check_Curve_DL_R(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 왼쪽위칸 뚫렸다면,
	if (Check_Under_Y(check_point.second - 1) && Check_Under_X(check_point.first - 1)) {
		if (tile_type[check_point.second - 1][check_point.first - 1] != eTILE_STATE::OBSTACLE) {
			// 위쪽 장애물 판단
			if (tile_type[check_point.second - 1][check_point.first] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}
	}

	return false;
}

// 0 x    
//   ↙ x
//      0
eSEARCH_RESULT Check_Curve_DL(const Pos check_point ) {
	if (Check_Obstacle(check_point))
		return eSEARCH_RESULT::UNABLE;
	if (!Check_Under_X(check_point.first))
		return eSEARCH_RESULT::UNABLE;
	if (!Check_Over_Y(check_point.second))
		return eSEARCH_RESULT::UNABLE;

	if (Check_Curve_DL_L(check_point))
		return  eSEARCH_RESULT::CORNER;
	if (Check_Curve_DL_R(check_point))
		return  eSEARCH_RESULT::CORNER;

	return eSEARCH_RESULT::NONE;
}

//   ↖
// 0  x
bool Check_Curve_UL_L(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 왼쪽아래칸 뚫렸다면,
	if (Check_Over_Y(check_point.second + 1) && Check_Under_X(check_point.first - 1)) {
		if (tile_type[check_point.second + 1][check_point.first - 1] != eTILE_STATE::OBSTACLE) {
			// 아래쪽 장애물 판단
			if (tile_type[check_point.second + 1][check_point.first] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}
	}

	return false;
}

//    0
// ↖ x
bool Check_Curve_UL_R(const Pos check_point) {
	// 도착점도 커브로 판단
	if (check_point == end_point)
		return true;

	// 오른쪽위칸 뚫렸다면,
	if (Check_Over_X(check_point.first + 1) && Check_Under_Y(check_point.second - 1)) {
		if (tile_type[check_point.second - 1][check_point.first + 1] != eTILE_STATE::OBSTACLE) {
			// 오른쪽 장애물 판단
			if (tile_type[check_point.second][check_point.first + 1] == eTILE_STATE::OBSTACLE) {
				return true;
			}
		}
	}

	return false;
}

//      0
//   ↖ x
// 0  x   
eSEARCH_RESULT Check_Curve_UL(Pos check_point) {
	if (Check_Obstacle(check_point))
		return eSEARCH_RESULT::UNABLE;
	if (!Check_Under_X(check_point.first))
		return eSEARCH_RESULT::UNABLE;
	if (!Check_Under_Y(check_point.second))
		return eSEARCH_RESULT::UNABLE;

	if (Check_Curve_UL_L(check_point))
		return  eSEARCH_RESULT::CORNER;
	if (Check_Curve_UL_R(check_point))
		return  eSEARCH_RESULT::CORNER;

	return eSEARCH_RESULT::NONE;
}

bool Search_Curve_UU(Pos node_pos, Pos* p_curve_pos) {
	for (;;) {
		node_pos.second--;

		switch (Check_Curve_UU(node_pos))
		{
			// winapi 디버그용
		case eSEARCH_RESULT::NONE: {
			Mark_SEARCH(node_pos);
			break;
		}

		case eSEARCH_RESULT::UNABLE: {
			return false;
			break;
		}

		case eSEARCH_RESULT::CORNER: {
			Mark_SEARCH(node_pos);
			*p_curve_pos = node_pos;
			return true;
			break;
		}

		default:
			throw;
		}
	}
}

bool Search_Curve_DD(Pos node_pos, Pos* p_curve_pos) {
	for (;;) {
		node_pos.second++;

		switch (Check_Curve_DD(node_pos)) {
			// winapi 디버그용
		case eSEARCH_RESULT::NONE: {
			Mark_SEARCH(node_pos);
			break;
		}

		case eSEARCH_RESULT::UNABLE: {
			return false;
			break;
		}

		case eSEARCH_RESULT::CORNER: {
			Mark_SEARCH(node_pos);
			*p_curve_pos = node_pos;
			return true;
			break;
		}

		default:
			throw;
		}
	}
}

bool Search_Curve_LL(Pos node_pos, Pos* p_curve_pos) {
	for (;;) {
		node_pos.first--;

		switch (Check_Curve_LL(node_pos))		{
				// winapi 디버그용
			case eSEARCH_RESULT::NONE: {
				Mark_SEARCH(node_pos);
				break;
			}

			case eSEARCH_RESULT::UNABLE: {
				return false;
				break;
			}

			case eSEARCH_RESULT::CORNER: {
				Mark_SEARCH(node_pos);
				*p_curve_pos = node_pos;
				return true;
				break;
			}

			default:
				throw;
		}
	}
}

bool Search_Curve_RR(Pos node_pos, Pos* p_curve_pos) {
	for (;;) {
		node_pos.first++;

		switch (Check_Curve_RR(node_pos))		{
				// winapi 디버그용
			case eSEARCH_RESULT::NONE: {
				Mark_SEARCH(node_pos);
				break;
			}

			case eSEARCH_RESULT::UNABLE: {
				return false;
				break;
			}

			case eSEARCH_RESULT::CORNER: {
				Mark_SEARCH(node_pos);
				*p_curve_pos = node_pos;
				return true;
				break;
			}

			default:
				throw;
		}
	}
}

bool Search_Curve_UL(Pos node_pos, Pos* p_curve_pos) {
	for (;;) {
		node_pos.first--;
		node_pos.second--;

		switch (Check_Curve_UL(node_pos)) {
			// winapi 디버그용
		case eSEARCH_RESULT::NONE: {
			Mark_SEARCH(node_pos);
			break;
		}

		case eSEARCH_RESULT::UNABLE: {
			return false;
			break;
		}

		case eSEARCH_RESULT::CORNER: {
			Mark_SEARCH(node_pos);
			*p_curve_pos = node_pos;
			return true;
			break;
		}

		default:
			throw;
		}

		if (Search_Curve_UU(node_pos, p_curve_pos)) {
			*p_curve_pos = node_pos;
			return true;
		}
		else if (Search_Curve_LL(node_pos, p_curve_pos)) {
			*p_curve_pos = node_pos;
			return true;
		}
	}
}

bool Search_Curve_UR(Pos node_pos, Pos* p_curve_pos) {
	for (;;) {
		node_pos.first++;
		node_pos.second--;

		switch (Check_Curve_UR(node_pos)) {
				// winapi 디버그용
			case eSEARCH_RESULT::NONE: {
				Mark_SEARCH(node_pos);
				break;
			}

			case eSEARCH_RESULT::UNABLE: {
				return false;
				break;
			}
									   
			case eSEARCH_RESULT::CORNER: {
				Mark_SEARCH(node_pos);
				*p_curve_pos = node_pos;
				return true;
				break;
			}

			default:
				throw;
		}

		if (Search_Curve_UU(node_pos, p_curve_pos)) {
			*p_curve_pos = node_pos;
			return true;
		}
		else if (Search_Curve_RR(node_pos, p_curve_pos)) {
			*p_curve_pos = node_pos;
			return true;
		}
	}
}

bool Search_Curve_DL(Pos node_pos, Pos* p_curve_pos) {
	for (;;) {
		node_pos.first--;
		node_pos.second++;

		switch (Check_Curve_DL(node_pos)) {
			// winapi 디버그용
		case eSEARCH_RESULT::NONE: {
			Mark_SEARCH(node_pos);
			break;
		}

		case eSEARCH_RESULT::UNABLE: {
			return false;
			break;
		}

		case eSEARCH_RESULT::CORNER: {
			Mark_SEARCH(node_pos);
			*p_curve_pos = node_pos;
			return true;
			break;
		}

		default:
			throw;
		}

		if (Search_Curve_DD(node_pos, p_curve_pos)) {
			*p_curve_pos = node_pos;
			return true;
		}
		else if (Search_Curve_LL(node_pos, p_curve_pos)) {
			*p_curve_pos = node_pos;
			return true;
		}
	}
}

bool Search_Curve_DR(Pos node_pos, Pos* p_curve_pos) {
	for (;;) {
		node_pos.first++;
		node_pos.second++;

		switch (Check_Curve_DR(node_pos)) {
			// winapi 디버그용
		case eSEARCH_RESULT::NONE: {
			Mark_SEARCH(node_pos);
			break;
		}

		case eSEARCH_RESULT::UNABLE: {
			return false;
			break;
		}

		case eSEARCH_RESULT::CORNER: {
			Mark_SEARCH(node_pos);
			*p_curve_pos = node_pos;
			return true;
			break;
		}

		default:
			throw;
		}

		if (Search_Curve_DD(node_pos, p_curve_pos)) {
			*p_curve_pos = node_pos;
			return true;
		}
		else if (Search_Curve_RR(node_pos, p_curve_pos)) {
			*p_curve_pos = node_pos;
			return true;
		}
	}
}

bool PushBack_OpenList(Node* const parent_node, Pos node_pos, eDIR node_dir) {
	if (Fine_close_list(node_pos))
		return false;

	// 대각선 체크
	bool is_diag = false;
	if (node_dir == eDIR::DIR_UL ||
		node_dir == eDIR::DIR_UR ||
		node_dir == eDIR::DIR_DL ||
		node_dir == eDIR::DIR_DR )
		is_diag = true;

	float cur_g;
	if (is_diag)
		cur_g = (Get_Distance(parent_node->pos, node_pos) / 2) * 1.5;
	else
		cur_g = Get_Distance(parent_node->pos, node_pos);
	cur_g += parent_node->g;

	// open list에 이미 있다면,
	Node* open_node;
	if (Fine_open_list(node_pos, &open_node)) {
		// 해당 노드 g, f, dir, parent 재설정
		if (cur_g < open_node->g) {
			open_node->g = cur_g;
			open_node->f = open_node->g + open_node->h;
			open_node->parent = parent_node;
			open_node->dir = node_dir;

			// 디버깅 코드
			ghf[node_pos.second][node_pos.first].g = open_node->g;
			ghf[node_pos.second][node_pos.first].h = open_node->h;
			ghf[node_pos.second][node_pos.first].f = open_node->f;
			ghf[node_pos.second][node_pos.first].flag_node = true;
		}
		tile_type[node_pos.second][node_pos.first] = eTILE_STATE::SCHEDULED;
	}
	// open list에 없다면.
	else {
		// 노드를 만들어서 open list에 넣는다.
		nodes[node_pos.second][node_pos.first].Set(node_pos, cur_g, Get_h(node_pos), parent_node, node_dir);
		open_list.emplace_back(&nodes[node_pos.second][node_pos.first]);
		tile_type[node_pos.second][node_pos.first] = eTILE_STATE::SCHEDULED;

		// 디버깅 코드
		ghf[node_pos.second][node_pos.first].g = nodes[node_pos.second][node_pos.first].g;
		ghf[node_pos.second][node_pos.first].h = nodes[node_pos.second][node_pos.first].h;
		ghf[node_pos.second][node_pos.first].f = nodes[node_pos.second][node_pos.first].f;
		ghf[node_pos.second][node_pos.first].flag_node = true;
		return true;
	}

	return false;
}

stRGB inital_rgb(0xDA70D6);
void OpenList_PushBack_Proc(Node* const p_parent_node) {
	inital_rgb += 0x0F0F0F;
	search_rgb.Set(inital_rgb);
	Pos pos;

	switch (p_parent_node->dir) {
	case eDIR::DIR_ALL:	{
		// 8방향
		if (Search_Curve_UU(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UU); }
		if (Search_Curve_RR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_RR); }
		if (Search_Curve_DD(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DD); }
		if (Search_Curve_LL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_LL); }

		if (Search_Curve_UR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UR); }
		if (Search_Curve_DR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DR); }
		if (Search_Curve_DL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DL); }
		if (Search_Curve_UL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UL); }
	}
	break;

	case eDIR::DIR_UU:	{
		if (Search_Curve_UU(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UU); }
		// 조건 부
		if (Check_Curve_UU_L(p_parent_node->pos)) 
			if (Search_Curve_UL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UL); }
		if (Check_Curve_UU_R(p_parent_node->pos)) 
			if (Search_Curve_UR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UR); }
	}
	break;

	case eDIR::DIR_DD:	{
		if (Search_Curve_DD(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DD); }
		// 조건 부
		if (Check_Curve_DD_L(p_parent_node->pos))
			if (Search_Curve_DR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DR); }
		if (Check_Curve_DD_R(p_parent_node->pos))
			if (Search_Curve_DL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DL); }
	}
	break;

	case eDIR::DIR_LL:	{
		if (Search_Curve_LL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_LL); }
		// 조건 부
		if (Check_Curve_LL_L(p_parent_node->pos))
			if (Search_Curve_DL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DL); }
		if (Check_Curve_LL_R(p_parent_node->pos))
			if (Search_Curve_UL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UL); }
	}
	break;

	case eDIR::DIR_RR:	{
		if (Search_Curve_RR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_RR); }
		// 조건 부
		if (Check_Curve_RR_L(p_parent_node->pos))
			if (Search_Curve_UR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UR); }
		if (Check_Curve_RR_R(p_parent_node->pos))
			if (Search_Curve_DR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DR); }
		break;
	}

	case eDIR::DIR_UL: {
		if (Search_Curve_UL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UL); }
		if (Search_Curve_UU(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UU); }
		if (Search_Curve_LL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_LL); }
		// 조건 부
		if (Check_Curve_UL_L(p_parent_node->pos))
			if (Search_Curve_DL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DL); }
		if (Check_Curve_UL_R(p_parent_node->pos))
			if (Search_Curve_UR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UR); }

		break;
	}

	case eDIR::DIR_UR:	{
		if (Search_Curve_UR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UR); }
		if (Search_Curve_UU(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UU); }
		if (Search_Curve_RR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_RR); }
		// 조건 부
		if (Check_Curve_UR_L(p_parent_node->pos))
			if (Search_Curve_UL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UL); }
		if (Check_Curve_UR_R(p_parent_node->pos))
			if (Search_Curve_DR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DR); }

		break;
	}

	case eDIR::DIR_DR:	{
		if (Search_Curve_DR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DR); }
		if (Search_Curve_DD(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DD); }
		if (Search_Curve_RR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_RR); }
		// 조건 부
		if (Check_Curve_DR_L(p_parent_node->pos))
			if (Search_Curve_UR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UR); }
		if (Check_Curve_DR_R(p_parent_node->pos))
			if (Search_Curve_DL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DL); }
	}
	break;

	case eDIR::DIR_DL:	{
		if (Search_Curve_DL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DL); }
		if (Search_Curve_DD(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DD); }
		if (Search_Curve_LL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_LL); }
		// 조건 부
		if (Check_Curve_DL_L(p_parent_node->pos))
			if (Search_Curve_DR(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_DR); }
		if (Check_Curve_DL_R(p_parent_node->pos))
			if (Search_Curve_UL(p_parent_node->pos, &pos)) { PushBack_OpenList(p_parent_node, pos, eDIR::DIR_UL); }
	}
	break;

	default:
		break;
	}

	open_list.sort(Compare_node_p);
}

bool JPS(Pos start, Pos end) {
	start_point = start;
	end_point = end;
	open_list.clear();
	close_list.clear();

	//디버깅 용
	for (int i = 0; i < GRID_HEIGHT * GRID_WIDTH; i++) {
		if (tile_type[0][i] == eTILE_STATE::VISITED || tile_type[0][i] == eTILE_STATE::SCHEDULED || tile_type[0][i] == eTILE_STATE::SEARCH)
			tile_type[0][i] = eTILE_STATE::NONE;
	}

	// 렌더 (winapi 디버깅 코드)
	Window_Clear();
	WM_PAINT_Proc();
	//Sleep(500);

	nodes[start.second][start.first].Set(start, 0, Get_h(start), nullptr, eDIR::DIR_ALL);
	open_list.emplace_back(&nodes[start.second][start.first]);

	for (;;) {
		// open_list가 비었음 (도달할 수 없는 경우)
		if (open_list.empty()) {
			break;
		}

		// 노드 뽑기 (Open list -> Close list 이동, 해당 노드 방문처리)
		Node* visit_node = open_list.front();
		open_list.pop_front();
		close_list.emplace_back(visit_node);
		//if(tile_type[visit_node->pos.second][visit_node->pos.first] != eTILE_STATE::SCHEDULED)
			tile_type[visit_node->pos.second][visit_node->pos.first] = eTILE_STATE::VISITED;

		// 도착판단
		if (visit_node->pos == end_point) {
			// winapi 디버깅용
			tile_type[start_point.second][start_point.first] = eTILE_STATE::START;
			tile_type[end_point.second][end_point.first] = eTILE_STATE::END;

			// 렌더 (winapi 디버깅 코드)
			WM_PAINT_Proc();
			return true;
		}

		// 타일 탐색 및 노드 추가
		OpenList_PushBack_Proc(visit_node);

		// 렌더 (winapi 디버깅 코드)
		//WM_PAINT_Proc();
		//Sleep(500);
	}

	return false;
}

// 디버깅 용
void Mark_SEARCH(Pos tile_pos) {
	//tile_type[tile_pos.second][tile_pos.first] = eTILE_STATE::SEARCH;
	//tile_color[tile_pos.second][tile_pos.first] = search_rgb;
}