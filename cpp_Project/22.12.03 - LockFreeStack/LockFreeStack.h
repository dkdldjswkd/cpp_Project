#pragma once
#include <Windows.h>
#include "ObjectPool.h"

// * �ش� LockFreeStack ���� ȯ�� : Release, x64, ����ȭ ������ OFF
//		J_LIB::ObjectPool�� ��������

#define CRASH()		do{						\
						int* p = nullptr;	\
						int a = *p;			\
					}while(true); 

template <typename T>
struct LockFreeStack{
public:
	struct Node {
	public:
		Node();
		Node(const T& data);
		~Node();

	public:
		T data;
		Node* p_next;

	public:
		void Clear();
	};

public:
	LockFreeStack();
	~LockFreeStack();

private:
	J_LIB::ObjectPool<Node> node_pool;

public:
	__declspec(align(64)) DWORD64 unique = 0;
	__declspec(align(64)) DWORD64 unique_top = NULL;
	__declspec(align(64)) int node_count = 0;

public:
	void Push(T data);
	bool Pop(T* dst);
	int GetUseCount();
};

//------------------------------
// LockFreeStack
//------------------------------

template <typename T>
LockFreeStack<T>::LockFreeStack() {
}

template <typename T>
LockFreeStack<T>::~LockFreeStack() {
	// ���� 17bit(unique ��) ����
	DWORD64 copy_unique_top = unique_top;
	Node* top = (Node*)((copy_unique_top << UNUSED_BIT) >> UNUSED_BIT);

	for (; top != nullptr;) {
		Node* delete_node = top;
		top = top->p_next;
		node_pool.Free(delete_node);
	}
}

template <typename T>
void LockFreeStack<T>::Push(T data) {
	// Create Node
	Node* insert_node = node_pool.Alloc();
	insert_node->Clear();
	insert_node->data = data;

	// Node�� �ּҸ� Unique�ϰ� �ٲ㼭 ���ÿ� ���� (ABA �̽� �ذ�å)
	DWORD64 unique_num = InterlockedIncrement64((LONG64*)&unique);
	DWORD64 unique_node = ((unique_num << (64 - UNUSED_BIT)) | (DWORD64)insert_node);

	for (;;) {
		DWORD64 copy_unique_top = unique_top;
		Node* copy_top = (Node*)((copy_unique_top << UNUSED_BIT) >> UNUSED_BIT);
		insert_node->p_next = copy_top;

		// ���ÿ� ��ȭ�� �־��ٸ� �ٽýõ�
		if ((DWORD64)copy_unique_top != InterlockedCompareExchange64((LONG64*)&unique_top, (LONG64)unique_node, (LONG64)copy_unique_top))
			continue;

		InterlockedIncrement((LONG*)&node_count);
		return;
	}
}

template <typename T>
bool LockFreeStack<T>::Pop(T* dst) {
	DWORD64 unique_num = InterlockedIncrement64((LONG64*)&unique);

	for (;;) {
		DWORD64 copy_unique_top = unique_top;
		Node* copy_top = (Node*)((copy_unique_top << UNUSED_BIT) >> UNUSED_BIT);

		// Not empty!!
		if (copy_top) {
			Node* unique_next = (Node*)((unique_num << (64 - UNUSED_BIT)) | (DWORD64)copy_top->p_next);

			// ���ÿ� ��ȭ�� �־��ٸ� �ٽýõ�
			if (copy_unique_top != InterlockedCompareExchange64((LONG64*)&unique_top, (LONG64)unique_next, (LONG64)copy_unique_top))
				continue;

			InterlockedDecrement((LONG*)&node_count);
			memmove(dst, &copy_top->data, sizeof(copy_top->data));
			node_pool.Free(copy_top);
			return true;
		}
		// empty!!
		else {
			dst = nullptr;
			return false;
		}
	}
}

template <typename T>
int LockFreeStack<T>::GetUseCount() {
	return node_count;
}

//------------------------------
// Node
//------------------------------

template <typename T>
LockFreeStack<T>::Node::Node() : p_next(nullptr) {
}

template <typename T>
LockFreeStack<T>::Node::Node(const T& data) : data(data), p_next(nullptr) {
}

template <typename T>
LockFreeStack<T>::Node::~Node() {
}

template<typename T>
void LockFreeStack<T>::Node::Clear() {
	p_next = nullptr;
}