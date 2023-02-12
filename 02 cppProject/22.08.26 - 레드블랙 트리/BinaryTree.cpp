#pragma once
#include "stdafx.h"
#include "BinaryTree.h"
#include "WndProc.h"

using namespace std;

#define p_root_node (nill_node.left)

RedBlackTree::RedBlackTree() : nill_node({}, & nill_node, BLACK, & nill_node, & nill_node) { 
	nill_node.flag_nill = true; 
	nill_node.data = INT_MAX;
}

HBRUSH h_red_brush = CreateSolidBrush(RGB(255, 0, 0));
HBRUSH h_black_brush = CreateSolidBrush(RGB(0, 0, 0));

// nill node도 그린다.
void RedBlackTree::Real_Print(Node* p_cur_node, int left, int right, int parent_x, int parent_y) {
	int x = (left + right) / 2;
	int y = parent_y + HEIGHT_GAP;

	HDC hdc;
	PAINTSTRUCT ps;
	hdc = GetDC(h_wnd);

	HBRUSH h_brush_old;
	if (p_cur_node->red)
		h_brush_old = (HBRUSH)SelectObject(hdc, h_red_brush);
	else
		h_brush_old = (HBRUSH)SelectObject(hdc, h_black_brush);

	// 부모 노드에 선 긋기
	if (p_cur_node != p_root_node) {
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, parent_x, parent_y);
	}

	// 노드 동그라미 그리기
	Ellipse(hdc, x - RADIUS, y - RADIUS, x + RADIUS, y + RADIUS);

	// (닐노드 제외) 데이터 출력
	if (!p_cur_node->flag_nill) {
		wchar_t buf[32];
		wsprintf(buf, L"%d", p_cur_node->data);
		RECT rect = { x - RECT_SIZE,y - RECT_SIZE,x + RECT_SIZE,y + RECT_SIZE };
		DrawText(hdc, buf, -1, &rect, DT_CENTER | DT_WORDBREAK);
	}
	else {
		goto end_recursive;
	}

	Real_Print(p_cur_node->left, left, x, x, y);
	Real_Print(p_cur_node->right, x, right, x, y);

end_recursive:
	SelectObject(hdc, h_brush_old);
	ReleaseDC(h_wnd, hdc);
	return;
}

void RedBlackTree::Print() {
	Real_Print(p_root_node, 0 + 40, WINDOW_WIDTH + 40, 0, 0);
}

void RedBlackTree::Son_is_black(Node* p_cur_node) {
		if (p_cur_node->flag_nill) return;

		if (p_cur_node->red == RED) {
			if (p_cur_node->left->red == RED) {
				throw;
			}
			if (p_cur_node->right->red == RED) {
				throw;
			}
		}

		Son_is_black(p_cur_node->left);
		Son_is_black(p_cur_node->right);
}

void RedBlackTree::Check_Black(Node* p_cur_node, int n, int parent_data) {
	// 리프 노드
	if (p_cur_node->flag_nill) {
		if (Number_of_black() == (n + 1)) {
		//	cout << "통과" << parent_data << "블랙 노드 개수 : " << Number_of_black() << endl;
			return;
		}
		else
			throw;
	}
	
	if (p_cur_node->red == BLACK) {
		Check_Black(p_cur_node->left, n + 1, p_cur_node->data);
		Check_Black(p_cur_node->right, n + 1, p_cur_node->data);
	}
	else {
		Check_Black(p_cur_node->left, n, p_cur_node->data);
		Check_Black(p_cur_node->right, n, p_cur_node->data);
	}
}

void RedBlackTree::Check_Black() {
	Check_Black(p_root_node, 0, 0);
	//printf("루트 노드로 부터 모든 리프 노드까지의 블랙 노드의 개수는 같다. \n");
}

void RedBlackTree::Son_is_black() {
	Son_is_black(p_root_node);
//	printf("모든 레드 노드의 자식은 블랙이다. \n");
}

int RedBlackTree::Number_of_black(Node* p_node) {
	if (p_node->flag_nill)
		return 1;

	if (p_node->red == BLACK)
		return 1 + Number_of_black(p_node->left);
	else
		return Number_of_black(p_node->left);
}

int RedBlackTree::Number_of_black() {
	return  Number_of_black(p_root_node);
}

bool RedBlackTree::Real_Insert(int data, Node* p_cur_node) {
	// 오른쪽 자식으로 삽입
	if (p_cur_node->data < data) {
		if (p_cur_node->right->flag_nill) {
			p_cur_node->right = Alloc_Node(data, p_cur_node, RED);
			Insert_Balance(p_cur_node->right);
			return true;
		}
		else {
			Real_Insert(data, p_cur_node->right);
		}
	}
	//왼쪽 자식으로 삽입
	else if (data <  p_cur_node->data ) {
		if (p_cur_node->left->flag_nill) {
			p_cur_node->left = Alloc_Node(data, p_cur_node, RED);
			Insert_Balance(p_cur_node->left);
			return true;
		}
		else {
			Real_Insert(data, p_cur_node->left);
		}
	}
	//같은 값 삽입
	else {
		return false;
	}
}

void RedBlackTree::Insert_Balance(Node* p_check_node) {
	if (p_check_node == p_root_node) {
		p_check_node->red = BLACK;
		return;
	}
	if (p_check_node->parent->red == BLACK)
		return;

	switch (p_check_node->GetUnclePointer()->red)	{
		case RED: {
			p_check_node->parent->red = BLACK;
			p_check_node->GetUnclePointer()->red = BLACK;
			p_check_node->GetGrandpaPointer()->red = RED;
			Insert_Balance(p_check_node->GetGrandpaPointer());
			break;
		}

		case BLACK: {
			// 좌회전
			if (p_check_node->parent->GetDir() == RIGHT) {
				if (p_check_node->GetDir() == RIGHT) {
					Rotate_Node(p_check_node->GetGrandpaPointer()->data, LEFT);
					p_check_node->parent->red = BLACK;
					p_check_node->parent->left->red = RED;
				}
				else {
					Rotate_Node(p_check_node->parent->data, RIGHT);
					Insert_Balance(p_check_node->right);
					return;
				}
			}
			// 우회전
			else if (p_check_node->parent->GetDir() == LEFT) {
				if (p_check_node->GetDir() == LEFT) {
					Rotate_Node(p_check_node->GetGrandpaPointer()->data, RIGHT);
					p_check_node->parent->red = BLACK;
					p_check_node->parent->right->red = RED;
				}
				else {
					Rotate_Node(p_check_node->parent->data, LEFT);
					Insert_Balance(p_check_node->left);
					return;
				}
			}
			break;
		}

		default:
			throw;
	}
}

void draw_white() {
	HDC hdc;
	hdc = GetDC(h_wnd);
	Rectangle(hdc, -30, -30, 1500, 1500);
	ReleaseDC(h_wnd, hdc);
}

void RedBlackTree::Remove_Balance(Node* p_balance_node) {
	/////////////////////////////////////////////
	// 대체 노드가 레드일때
	/////////////////////////////////////////////

	if (p_balance_node->red == RED) {
		// 대체 노드를 블랙으로 칠하여 밸런스 (* 문제 해결)
		p_balance_node->red = BLACK;
		return;
	}

	/////////////////////////////////////////////
	// 대체 노드가 블랙일때 (이중 블랙 노드)
	/////////////////////////////////////////////

	int dir = p_balance_node->GetDir();

	// 이중 블랙노드가 왼쪽일때
	if (dir == LEFT) {

		///////////////////////////////////////////////////////////////////////////////////////
		//	이중 블랙노드의 형제가 레드일때 (형제를 블랙으로 만드는 사전작업을 해줌.)
		///////////////////////////////////////////////////////////////////////////////////////
		
		if (p_balance_node->GetSiblingPointer()->red == RED) {
			// 형제와 부모색 교환
			p_balance_node->GetSiblingPointer()->red = BLACK;
			p_balance_node->parent->red = RED; // 문제없음

			// 이중 블랙노드의 방향으로 회전
			Rotate_Node(p_balance_node->parent->data, LEFT);

			// 디버깅
			draw_white();
			Print();

			// 이중 블랙 노드 형제의 색이 블랙인 상황으로 만들었음 (depth 한단계 깊어짐)
			// 다시 밸런스 작업 진행
			Remove_Balance(p_balance_node);
		}

		///////////////////////////////////////////////////////////////////////////////////////
		//	이중 블랙노드의 형제가 블랙일때
		///////////////////////////////////////////////////////////////////////////////////////

		else {
			// 형제의 양 자식이 블랙인 경우
			if (p_balance_node->GetSiblingPointer()->left->red == BLACK &&
				p_balance_node->GetSiblingPointer()->right->red == BLACK)
			{
				// 형제를 레드로 바꾸고
				 p_balance_node->GetSiblingPointer()->red = RED; // 문제있음 닐노드 빨간색 됨

				// 디버깅
				draw_white();
				Print();

				// 이중 블랙노드의 블랙을 하나 부모에게 넘긴다.
				Remove_Balance(p_balance_node->parent);
			}

			// 형제의 왼쪽 자식이 레드, 오른쪽 자식이 블랙인 경우
			else if (p_balance_node->GetSiblingPointer()->left->red == RED &&
				p_balance_node->GetSiblingPointer()->right->red == BLACK)
			{
				// 형제와 부모의 색 교환
				p_balance_node->GetSiblingPointer()->left->red = BLACK;
				p_balance_node->GetSiblingPointer()->red = RED;

				// 형제의 오른쪽(블랙) 자식쪽으로 회전
				Rotate_Node(p_balance_node->GetSiblingPointer()->data, RIGHT);

				// 디버깅
				draw_white();
				Print();

				Remove_Balance(p_balance_node);
			}

			// 형제의 오른쪽 자식이 레드인 경우
			else if (p_balance_node->GetSiblingPointer()->right->red == RED)
			{
				// 형제와 부모의 색 교환
				p_balance_node->GetSiblingPointer()->red = p_balance_node->parent->red;
				p_balance_node->parent->red = BLACK;
				// 형제의 블랙을 뺏어왔으니 형제의 오른자식 검은칠
				p_balance_node->GetSiblingPointer()->right->red = BLACK;

				// 이중 블랙노드 방향으로 회전 (* 문제해결)
				Rotate_Node(p_balance_node->parent->data, LEFT);

				// 디버깅
				draw_white();
				Print();
				return;
			}
			else {
				throw;
			}
		}
	}

	// 이중 블랙노드가 오른쪽일때
	else {

		///////////////////////////////////////////////////////////////////////////////////////
		//	이중 블랙노드의 형제가 레드일때 (형제를 블랙으로 만드는 사전작업을 해줌.)
		///////////////////////////////////////////////////////////////////////////////////////

		if (p_balance_node->GetSiblingPointer()->red == RED) {
			// 형제와 부모색 교환
			p_balance_node->GetSiblingPointer()->red = BLACK;
			p_balance_node->parent->red = RED;

			// 이중 블랙노드의 방향으로 회전
			Rotate_Node(p_balance_node->parent->data, RIGHT);

			// 디버깅
			draw_white();
			Print();

			// 이중 블랙 노드 형제의 색이 블랙인 상황으로 만들었음 (depth 한단계 깊어짐)
			// 다시 밸런스 작업 진행
			Remove_Balance(p_balance_node);
		}

		///////////////////////////////////////////////////////////////////////////////////////
		//	이중 블랙노드의 형제가 블랙일때
		///////////////////////////////////////////////////////////////////////////////////////

		else {
			// 형제의 양 자식이 블랙인 경우
			if (p_balance_node->GetSiblingPointer()->left->red == BLACK &&
				p_balance_node->GetSiblingPointer()->right->red == BLACK)
			{
				// 형제를 레드로 바꾸고
				p_balance_node->GetSiblingPointer()->red = RED;

				// 디버깅
				draw_white();
				Print();

				// 이중 블랙노드의 블랙을 하나 부모에게 넘긴다.
				Remove_Balance(p_balance_node->parent);
			}

			// 형제의 왼쪽 자식이 레드, 오른쪽 자식이 블랙인 경우
			else if (p_balance_node->GetSiblingPointer()->right->red == RED &&
				p_balance_node->GetSiblingPointer()->left->red == BLACK)
			{
				// 형제와 부모의 색 교환
				p_balance_node->GetSiblingPointer()->red = RED;
				p_balance_node->GetSiblingPointer()->right->red = BLACK;

				// 형제의 오른쪽(블랙) 자식쪽으로 회전
				Rotate_Node(p_balance_node->GetSiblingPointer()->data, LEFT);

				// 디버깅
				draw_white();
				Print();

				Remove_Balance(p_balance_node);
			}

			// 형제의 왼쪽 자식이 레드인 경우
			else if (p_balance_node->GetSiblingPointer()->left->red == RED)
			{
				// 형제와 부모의 색 교환
				p_balance_node->GetSiblingPointer()->red = p_balance_node->parent->red;
				p_balance_node->parent->red = BLACK;
				// 형제의 블랙을 뺏어왔으니 형제의 왼자식 검은칠
				p_balance_node->GetSiblingPointer()->left->red = BLACK;

				// 디버깅
				draw_white();
				Print();

				// 이중 블랙노드 방향으로 회전 (* 문제해결)
				Rotate_Node(p_balance_node->parent->data, RIGHT);
				return;
			}
		}
	}
}

///////////////////////////////////////////////////////////////
// 삭제 정책 : 삭제하려는 노드의 오른쪽 자식의 왼쪽 끝 자식
///////////////////////////////////////////////////////////////
bool RedBlackTree::Remove_Node(int data) {
	Node* p_node = Get_Node(data);
	if (p_node != nullptr) {
		Real_Remove(p_node);
		return true;
	}
	else
		return false;
}

void RedBlackTree::Rotate_Node(int data, bool dir) {
	Node* p_node = Get_Node(data);
	if (p_node == nullptr)
		return;

	// 좌회전
	if (dir) {
		if (p_node->right->flag_nill)
			return;

		// 내 부모노드의 자식 재설정
		if (p_node->GetDir() == LEFT) {
			p_node->parent->left = p_node->right;
		}
		else {
			p_node->parent->right = p_node->right;
		}

		// 회전 기준 노드의 자식의 depth를 -1 하는 과정
		// 자식의 부모 재설정 (기준노드의 부모로)
		p_node->right->parent = p_node->parent;
		// 기준노드의 부모 재설정 (기준노드의 부모를 자식 노드로)
		p_node->parent = p_node->right;

		// 범프 노드 연결 (기준 노드의 새로운 부모가 되는 노드는 )
		if (!p_node->parent->left->flag_nill) {
			p_node->parent->left->parent = p_node;
			p_node->right = p_node->parent->left;
		}
		else {
			p_node->right = &nill_node;
		}

		// 새로운 부모의 자식을 해당노드로
		p_node->parent->left = p_node;
	}
	// 우 회전
	else {
		if (p_node->left->flag_nill)
			return;

		// 부모노드 자식 재설정
		if (p_node->GetDir() == LEFT)
			p_node->parent->left = p_node->left;
		else
			p_node->parent->right = p_node->left;

		p_node->left->parent = p_node->parent;
		p_node->parent = p_node->left;

		// 범프 노드 연결
		if (!p_node->parent->right->flag_nill) {
			p_node->parent->right->parent = p_node;
			p_node->left = p_node->parent->right;
		}
		else {
			p_node->left = &nill_node;
		}

		// 새로운 부모의 자식을 해당노드로
		p_node->parent->right = p_node;
	}
}

Node* RedBlackTree::Get_Node(int data){
	Node* p_cur_node = p_root_node;

	for (;;) {
		if (p_cur_node->flag_nill)
			return nullptr;

		if (p_cur_node->data < data)
			p_cur_node = p_cur_node->right;
		else if (data < p_cur_node->data)
			p_cur_node = p_cur_node->left;
		else
			return p_cur_node;
	}
}

// 루트 노드 삭제 해보기
void RedBlackTree::Real_Remove(Node* p_delete_node) {
	// 삭제 대상 조건에 만족하는지,
	if (!p_delete_node->left->flag_nill && !p_delete_node->right->flag_nill) {
		// 삭제 대상 변경
		Node* p_cur_node = p_delete_node->right;
		while (!p_cur_node->left->flag_nill) {
			p_cur_node = p_cur_node->left;
		}
		p_delete_node->data = p_cur_node->data;

		// 변경된 삭제 대상으로 재귀
		Real_Remove(p_cur_node);
		return;
	}

	Node* p_replace_node;

	// 삭제노드의 왼쪽 자식만 존재
	if (!p_delete_node->left->flag_nill && p_delete_node->right->flag_nill) {
		if (p_delete_node->GetDir() == LEFT) {
			p_delete_node->parent->left = p_delete_node->left;
			p_delete_node->left->parent = p_delete_node->parent;
		}
		else if (p_delete_node->GetDir() == RIGHT) {
			p_delete_node->parent->right = p_delete_node->left;
			p_delete_node->left->parent = p_delete_node->parent;
		}

		p_replace_node = p_delete_node->left;
	}
	// 삭제노드의 오른쪽 자식만 존재 (또는 자식이 없음)
	else{
		if (p_delete_node->GetDir() == LEFT) {
			p_delete_node->parent->left = p_delete_node->right;
			p_delete_node->right->parent = p_delete_node->parent;
		}
		else if (p_delete_node->GetDir() == RIGHT) {
			p_delete_node->parent->right = p_delete_node->right;
			p_delete_node->right->parent = p_delete_node->parent;
		}

		p_replace_node = p_delete_node->right;
	}

	// 대체 노드 밸런스 작업
	if (p_delete_node->red == BLACK) {
		Remove_Balance(p_replace_node);
	}
	delete p_delete_node;
}

bool RedBlackTree::Insert(int data) {
	return Real_Insert(data, p_root_node);
}