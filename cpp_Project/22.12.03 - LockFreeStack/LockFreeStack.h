#pragma once
#include <Windows.h>

#define TEST_LOOP 1000000
#define THREAD_NUM 3
#define NOTE_BYTE 1000000000


struct MemoryNote {
public:
	MemoryNote() {
		begin = (char*)malloc(NOTE_BYTE * sizeof(char));
		wp = begin - 8; // 8byte 씩 write 하기 위해 - 8
		end = begin + NOTE_BYTE;
	}

public:
	char* begin;
	char* wp;
	char* end;

public:
	inline char* get_wp() {
		return (char*)InterlockedAdd64((long long*)&wp, 8);
	}
};

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
	void Push(BYTE flag);
	void Pop(BYTE flag);
};