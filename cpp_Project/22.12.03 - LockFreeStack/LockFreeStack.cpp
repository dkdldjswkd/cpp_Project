#include <iostream>
#include "LockFreeStack.h"
#include <Windows.h>

//#define WRITE_NOTE(X) do{ *((char*)InterlockedIncrement64((long long*)&note.wp)) = X ;}while(false)

#define LOG(X)  do{ wp = note.get_wp(); *wp = X + flag; }while(false)
#define LOG_(X) do{ wp = note.get_wp(); memmove(wp + 1, &copy_top, 7); *wp = X + flag; }while(false)
#define LOG__(X) do{ wp = note.get_wp(); memmove(wp + 1, &insert_node, 7); *wp = X + flag; }while(false)

using namespace std;

MemoryNote note;

#define CRASH()		do{						\
						int* p = nullptr;	\
						int a = *p;			\
					}while(true); 

LockFreeStack::LockFreeStack(){
}

LockFreeStack::~LockFreeStack(){
}

void LockFreeStack::Push(BYTE flag) {
	int v = rand() % 1000;
	char* wp;
	Node* insert_node = new Node(v);
	LOG__(0);

	for (;;) {
		Node* copy_top = top;
		LOG_(1);

		insert_node->p_next = copy_top;
		LOG(2);

		// 스택에 변화가 없었다면,
		if (copy_top == (Node*)InterlockedCompareExchange64((LONG64*)&top, (LONG64)insert_node, (LONG64)copy_top)) {
			InterlockedIncrement((LONG*)&node_count);
			LOG(3);
			if (node_count > TEST_LOOP * THREAD_NUM) { printf("푸시 크래시 \n"); CRASH(); }
			break;
		}
		else {
			LOG(4);
		}
	}
}

void LockFreeStack::Pop(BYTE flag) {
	char* wp;
	Node* p;

	for (;;) {
		Node* copy_top = top;
		if (copy_top == nullptr) return;
		LOG_(5);

		Node* new_top = copy_top->p_next;
		LOG(6);

		// 스택에 변화가 없었다면,
		if (copy_top == (Node*)InterlockedCompareExchange64((LONG64*)&top, (LONG64)new_top, (LONG64)copy_top)) {
			InterlockedDecrement((LONG*)&node_count);

			if (node_count < 0) { printf("팝 크래시 \n"); CRASH(); }
			p = copy_top;
			LOG_(7);
			delete copy_top;
			break;
		}
		else {
			LOG(8);
		}
	}
}


//------------------------------
// Node
//------------------------------

LockFreeStack::Node::Node() {
}

LockFreeStack::Node::Node(int v) : v(v) {
}

LockFreeStack::Node::~Node(){
}
