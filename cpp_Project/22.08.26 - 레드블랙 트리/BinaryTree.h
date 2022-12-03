#pragma once
#include <stdio.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800

#define HEIGHT_GAP 50

#define RADIUS		10
#define RECT_SIZE	20

#define RED true
#define BLACK false

#define LEFT	true
#define RIGHT	false
#define NEITHER 3


struct Node {
public:
	Node(int data, Node* parent, bool red = true, Node* left = nullptr, Node* right = nullptr) : data(data), parent(parent), red(red), left(left), right(right) {}
	~Node() {}

public:
	int data;
	bool red;
	bool flag_nill = false;

	Node* parent;
	Node* left;
	Node* right;

public:
	inline Node* GetGrandpaPointer() { return parent->parent; }

	// left == 1, right == 0
	bool GetDir() {
		if (parent->left == this)
			return LEFT;
		else
			return RIGHT;
	}

	inline Node* GetUnclePointer() {
		int parent_dir = parent->GetDir();

		if (parent_dir == LEFT)
			return GetGrandpaPointer()->right;
		else if (parent_dir == RIGHT)
			return GetGrandpaPointer()->left;
		else
			return nullptr;
	}

	inline Node* GetSiblingPointer() {
		int dir = GetDir();

		if (dir == LEFT)
			return parent->right;
		else
			return parent->left;
	}

};

class RedBlackTree {
public:
	RedBlackTree();
	~RedBlackTree() {}

public:
	Node nill_node;
	#define p_root_node (nill_node.left)

private:
	void Real_Print(Node* p_curNode, int left, int right, int parent_x, int parent_y);
	bool Real_Insert(int data, Node* p_cur_node);
	void Real_Remove(Node* p_node);
	void Insert_Balance(Node* p_check_node);
	void Remove_Balance(Node* p_check_node);
	void Rotate_Node(int data, bool dir);
	Node* Get_Node(int data);
	inline Node* Alloc_Node(int data, Node* p_parent_node, bool color) {
		return new Node(data, p_parent_node, RED, &nill_node, &nill_node);
	}

public:
	bool Remove_Node(int data);
	bool Insert(int data);
	void Print();

public:
	// 레드 노드의 자식은 검은색 이어야 한다
	void Son_is_black(Node* p_cur_node);
	void Son_is_black();

	// 루트 노드로 부터 모든 리프 노드까지의 블랙 노드의 개수가 같아야 한다.
	int Number_of_black(Node* p_node);
	int Number_of_black();
	void Check_Black(Node* p_cur_node, int n, int parent_data);
	void Check_Black();
};
#undef p_root_node

void draw_white();
