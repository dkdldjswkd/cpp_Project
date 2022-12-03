#pragma once
#define TEST_LOOP 100000
#define THREAD_NUM 3

struct LockFreeStack {
public:
	struct Node {
	public:
		Node();
		Node(int v);
		~Node();

	public:
		int v;
		Node* p_next;
	};

public:
	LockFreeStack();
	~LockFreeStack();

public:
	Node* top = nullptr;
	int node_count = 0;

public:
	void Push(int v);
	void Pop();
};