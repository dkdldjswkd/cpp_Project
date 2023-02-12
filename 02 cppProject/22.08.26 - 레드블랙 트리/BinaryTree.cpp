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

// nill node�� �׸���.
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

	// �θ� ��忡 �� �߱�
	if (p_cur_node != p_root_node) {
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, parent_x, parent_y);
	}

	// ��� ���׶�� �׸���
	Ellipse(hdc, x - RADIUS, y - RADIUS, x + RADIUS, y + RADIUS);

	// (�ҳ�� ����) ������ ���
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
	// ���� ���
	if (p_cur_node->flag_nill) {
		if (Number_of_black() == (n + 1)) {
		//	cout << "���" << parent_data << "�� ��� ���� : " << Number_of_black() << endl;
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
	//printf("��Ʈ ���� ���� ��� ���� �������� �� ����� ������ ����. \n");
}

void RedBlackTree::Son_is_black() {
	Son_is_black(p_root_node);
//	printf("��� ���� ����� �ڽ��� ���̴�. \n");
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
	// ������ �ڽ����� ����
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
	//���� �ڽ����� ����
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
	//���� �� ����
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
			// ��ȸ��
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
			// ��ȸ��
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
	// ��ü ��尡 �����϶�
	/////////////////////////////////////////////

	if (p_balance_node->red == RED) {
		// ��ü ��带 ������ ĥ�Ͽ� �뷱�� (* ���� �ذ�)
		p_balance_node->red = BLACK;
		return;
	}

	/////////////////////////////////////////////
	// ��ü ��尡 ���϶� (���� �� ���)
	/////////////////////////////////////////////

	int dir = p_balance_node->GetDir();

	// ���� ����尡 �����϶�
	if (dir == LEFT) {

		///////////////////////////////////////////////////////////////////////////////////////
		//	���� ������� ������ �����϶� (������ ������ ����� �����۾��� ����.)
		///////////////////////////////////////////////////////////////////////////////////////
		
		if (p_balance_node->GetSiblingPointer()->red == RED) {
			// ������ �θ�� ��ȯ
			p_balance_node->GetSiblingPointer()->red = BLACK;
			p_balance_node->parent->red = RED; // ��������

			// ���� ������� �������� ȸ��
			Rotate_Node(p_balance_node->parent->data, LEFT);

			// �����
			draw_white();
			Print();

			// ���� �� ��� ������ ���� ���� ��Ȳ���� ������� (depth �Ѵܰ� �����)
			// �ٽ� �뷱�� �۾� ����
			Remove_Balance(p_balance_node);
		}

		///////////////////////////////////////////////////////////////////////////////////////
		//	���� ������� ������ ���϶�
		///////////////////////////////////////////////////////////////////////////////////////

		else {
			// ������ �� �ڽ��� ���� ���
			if (p_balance_node->GetSiblingPointer()->left->red == BLACK &&
				p_balance_node->GetSiblingPointer()->right->red == BLACK)
			{
				// ������ ����� �ٲٰ�
				 p_balance_node->GetSiblingPointer()->red = RED; // �������� �ҳ�� ������ ��

				// �����
				draw_white();
				Print();

				// ���� ������� ���� �ϳ� �θ𿡰� �ѱ��.
				Remove_Balance(p_balance_node->parent);
			}

			// ������ ���� �ڽ��� ����, ������ �ڽ��� ���� ���
			else if (p_balance_node->GetSiblingPointer()->left->red == RED &&
				p_balance_node->GetSiblingPointer()->right->red == BLACK)
			{
				// ������ �θ��� �� ��ȯ
				p_balance_node->GetSiblingPointer()->left->red = BLACK;
				p_balance_node->GetSiblingPointer()->red = RED;

				// ������ ������(��) �ڽ������� ȸ��
				Rotate_Node(p_balance_node->GetSiblingPointer()->data, RIGHT);

				// �����
				draw_white();
				Print();

				Remove_Balance(p_balance_node);
			}

			// ������ ������ �ڽ��� ������ ���
			else if (p_balance_node->GetSiblingPointer()->right->red == RED)
			{
				// ������ �θ��� �� ��ȯ
				p_balance_node->GetSiblingPointer()->red = p_balance_node->parent->red;
				p_balance_node->parent->red = BLACK;
				// ������ ���� ��������� ������ �����ڽ� ����ĥ
				p_balance_node->GetSiblingPointer()->right->red = BLACK;

				// ���� ����� �������� ȸ�� (* �����ذ�)
				Rotate_Node(p_balance_node->parent->data, LEFT);

				// �����
				draw_white();
				Print();
				return;
			}
			else {
				throw;
			}
		}
	}

	// ���� ����尡 �������϶�
	else {

		///////////////////////////////////////////////////////////////////////////////////////
		//	���� ������� ������ �����϶� (������ ������ ����� �����۾��� ����.)
		///////////////////////////////////////////////////////////////////////////////////////

		if (p_balance_node->GetSiblingPointer()->red == RED) {
			// ������ �θ�� ��ȯ
			p_balance_node->GetSiblingPointer()->red = BLACK;
			p_balance_node->parent->red = RED;

			// ���� ������� �������� ȸ��
			Rotate_Node(p_balance_node->parent->data, RIGHT);

			// �����
			draw_white();
			Print();

			// ���� �� ��� ������ ���� ���� ��Ȳ���� ������� (depth �Ѵܰ� �����)
			// �ٽ� �뷱�� �۾� ����
			Remove_Balance(p_balance_node);
		}

		///////////////////////////////////////////////////////////////////////////////////////
		//	���� ������� ������ ���϶�
		///////////////////////////////////////////////////////////////////////////////////////

		else {
			// ������ �� �ڽ��� ���� ���
			if (p_balance_node->GetSiblingPointer()->left->red == BLACK &&
				p_balance_node->GetSiblingPointer()->right->red == BLACK)
			{
				// ������ ����� �ٲٰ�
				p_balance_node->GetSiblingPointer()->red = RED;

				// �����
				draw_white();
				Print();

				// ���� ������� ���� �ϳ� �θ𿡰� �ѱ��.
				Remove_Balance(p_balance_node->parent);
			}

			// ������ ���� �ڽ��� ����, ������ �ڽ��� ���� ���
			else if (p_balance_node->GetSiblingPointer()->right->red == RED &&
				p_balance_node->GetSiblingPointer()->left->red == BLACK)
			{
				// ������ �θ��� �� ��ȯ
				p_balance_node->GetSiblingPointer()->red = RED;
				p_balance_node->GetSiblingPointer()->right->red = BLACK;

				// ������ ������(��) �ڽ������� ȸ��
				Rotate_Node(p_balance_node->GetSiblingPointer()->data, LEFT);

				// �����
				draw_white();
				Print();

				Remove_Balance(p_balance_node);
			}

			// ������ ���� �ڽ��� ������ ���
			else if (p_balance_node->GetSiblingPointer()->left->red == RED)
			{
				// ������ �θ��� �� ��ȯ
				p_balance_node->GetSiblingPointer()->red = p_balance_node->parent->red;
				p_balance_node->parent->red = BLACK;
				// ������ ���� ��������� ������ ���ڽ� ����ĥ
				p_balance_node->GetSiblingPointer()->left->red = BLACK;

				// �����
				draw_white();
				Print();

				// ���� ����� �������� ȸ�� (* �����ذ�)
				Rotate_Node(p_balance_node->parent->data, RIGHT);
				return;
			}
		}
	}
}

///////////////////////////////////////////////////////////////
// ���� ��å : �����Ϸ��� ����� ������ �ڽ��� ���� �� �ڽ�
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

	// ��ȸ��
	if (dir) {
		if (p_node->right->flag_nill)
			return;

		// �� �θ����� �ڽ� �缳��
		if (p_node->GetDir() == LEFT) {
			p_node->parent->left = p_node->right;
		}
		else {
			p_node->parent->right = p_node->right;
		}

		// ȸ�� ���� ����� �ڽ��� depth�� -1 �ϴ� ����
		// �ڽ��� �θ� �缳�� (���س���� �θ��)
		p_node->right->parent = p_node->parent;
		// ���س���� �θ� �缳�� (���س���� �θ� �ڽ� ����)
		p_node->parent = p_node->right;

		// ���� ��� ���� (���� ����� ���ο� �θ� �Ǵ� ���� )
		if (!p_node->parent->left->flag_nill) {
			p_node->parent->left->parent = p_node;
			p_node->right = p_node->parent->left;
		}
		else {
			p_node->right = &nill_node;
		}

		// ���ο� �θ��� �ڽ��� �ش����
		p_node->parent->left = p_node;
	}
	// �� ȸ��
	else {
		if (p_node->left->flag_nill)
			return;

		// �θ��� �ڽ� �缳��
		if (p_node->GetDir() == LEFT)
			p_node->parent->left = p_node->left;
		else
			p_node->parent->right = p_node->left;

		p_node->left->parent = p_node->parent;
		p_node->parent = p_node->left;

		// ���� ��� ����
		if (!p_node->parent->right->flag_nill) {
			p_node->parent->right->parent = p_node;
			p_node->left = p_node->parent->right;
		}
		else {
			p_node->left = &nill_node;
		}

		// ���ο� �θ��� �ڽ��� �ش����
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

// ��Ʈ ��� ���� �غ���
void RedBlackTree::Real_Remove(Node* p_delete_node) {
	// ���� ��� ���ǿ� �����ϴ���,
	if (!p_delete_node->left->flag_nill && !p_delete_node->right->flag_nill) {
		// ���� ��� ����
		Node* p_cur_node = p_delete_node->right;
		while (!p_cur_node->left->flag_nill) {
			p_cur_node = p_cur_node->left;
		}
		p_delete_node->data = p_cur_node->data;

		// ����� ���� ������� ���
		Real_Remove(p_cur_node);
		return;
	}

	Node* p_replace_node;

	// ��������� ���� �ڽĸ� ����
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
	// ��������� ������ �ڽĸ� ���� (�Ǵ� �ڽ��� ����)
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

	// ��ü ��� �뷱�� �۾�
	if (p_delete_node->red == BLACK) {
		Remove_Balance(p_replace_node);
	}
	delete p_delete_node;
}

bool RedBlackTree::Insert(int data) {
	return Real_Insert(data, p_root_node);
}