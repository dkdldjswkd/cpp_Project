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

		// ���ÿ� ��ȭ�� �����ٸ�,
		if (copy_top == (Node*)InterlockedCompareExchange64((LONG64*)&top, (LONG64)insert_node, (LONG64)copy_top)) {
			// �����
			InterlockedIncrement((LONG*)&node_count);

			if (node_count > TEST_LOOP * THREAD_NUM) {
				printf("Ǫ�� ũ���� \n");
				CRASH();
			}
			//else {
			//	printf("Ǫ�� %d \n", v);
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

		// ���ÿ� ��ȭ�� �����ٸ�,
		if (copy_top == (Node*)InterlockedCompareExchange64((LONG64*)&top, (LONG64)new_top, (LONG64)copy_top)) {
			// �����
			InterlockedDecrement((LONG*)&node_count);

			if (node_count < 0) {
				printf("�� ũ���� \n");
				CRASH();
			}
			//else {
			//	printf("�� %d \n", copy_top->v);
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
