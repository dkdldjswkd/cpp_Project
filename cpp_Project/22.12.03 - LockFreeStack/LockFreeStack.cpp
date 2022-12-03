#include <iostream>
#include "LockFreeStack.h"
#include <Windows.h>

using namespace std;

#define CRASH()		do{						\
						int* p = nullptr;	\
						int a = *p;			\
					}while(true); 

LockFreeStack::LockFreeStack(){
}

LockFreeStack::~LockFreeStack(){
}

void LockFreeStack::Push(int v) {
	Node* insert_node = new Node(v);

	for (;;) {
		Node* copy_top = top;
		insert_node->p_next = copy_top;

		// 스택에 변화가 없었다면,
		if (copy_top == (Node*)InterlockedCompareExchange64((LONG64*)&top, (LONG64)insert_node, (LONG64)copy_top)) {
			// 디버깅
			InterlockedIncrement((LONG*)&node_count);

			if (node_count > TEST_LOOP * THREAD_NUM) {
				printf("푸시 크래시 \n");
				CRASH();
			}
			//else {
			//	printf("푸시 %d \n", v);
			//}

			break;
		}
	}
}

void LockFreeStack::Pop() {
	for (;;) {
		if (top == nullptr)
			return;

		Node* copy_top = top;
		Node* new_top = copy_top->p_next;

		// 스택에 변화가 없었다면,
		if (copy_top == (Node*)InterlockedCompareExchange64((LONG64*)&top, (LONG64)new_top, (LONG64)copy_top)) {
			// 디버깅
			InterlockedDecrement((LONG*)&node_count);

			if (node_count < 0) {
				printf("팝 크래시 \n");
				CRASH();
			}
			//else {
			//	printf("팝 %d \n", copy_top->v);
			//}

			delete copy_top;
			break;
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
