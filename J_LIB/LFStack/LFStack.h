#pragma once
#include <Windows.h>
#include "LFObjectPool.h"
// LFStack - 원본

// * 해당 LFStack 구현 환경 : Release, x64, 최적화 컴파일 OFF
//	오후 10:48 2022-12-15

// J_LIB::LFObjectPool에 종속적임

#define CRASH()		do{						\
						int* p = nullptr;	\
						int a = *p;			\
					}while(true); 

template <typename T>
struct LFStack {
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
	LFStack();
	~LFStack();

private:
	J_LIB::LFObjectPool<Node> node_pool;

private:
	__declspec(align(64)) DWORD64 unique = 0;
	__declspec(align(64)) DWORD64 unique_top = NULL;
	__declspec(align(64)) int node_count = 0;

public:
	void Push(T data);
	bool Pop(T* dst);
	int GetUseCount();
};

//------------------------------
// LFStack
//------------------------------

template <typename T>
LFStack<T>::LFStack() {
}

template <typename T>
LFStack<T>::~LFStack() {
	// 상위 17bit(unique 값) 날림
	DWORD64 copy_unique_top = unique_top;
	Node* top = (Node*)((copy_unique_top << UNUSED_BIT) >> UNUSED_BIT);

	for (; top != nullptr;) {
		Node* delete_node = top;
		top = top->p_next;
		node_pool.Free(delete_node);
	}
}

template <typename T>
void LFStack<T>::Push(T data) {
	// Create Node
	Node* insert_node = node_pool.Alloc();
	insert_node->Clear();
	insert_node->data = data;

	// Node의 주소를 Unique하게 바꿔서 스택에 꼽음 (ABA 이슈 해결책)
	DWORD64 unique_num = InterlockedIncrement64((LONG64*)&unique);
	DWORD64 unique_node = ((unique_num << (64 - UNUSED_BIT)) | (DWORD64)insert_node);

	for (;;) {
		DWORD64 copy_unique_top = unique_top;
		Node* copy_top = (Node*)((copy_unique_top << UNUSED_BIT) >> UNUSED_BIT);
		insert_node->p_next = copy_top;

		// 스택에 변화가 있었다면 다시시도
		if ((DWORD64)copy_unique_top != InterlockedCompareExchange64((LONG64*)&unique_top, (LONG64)unique_node, (LONG64)copy_unique_top))
			continue;

		InterlockedIncrement((LONG*)&node_count);
		return;
	}
}

template <typename T>
bool LFStack<T>::Pop(T* dst) {
	DWORD64 unique_num = InterlockedIncrement64((LONG64*)&unique);

	for (;;) {
		DWORD64 copy_unique_top = unique_top;
		Node* copy_top = (Node*)((copy_unique_top << UNUSED_BIT) >> UNUSED_BIT);

		// Not empty!!
		if (copy_top) {
			Node* unique_next = (Node*)((unique_num << (64 - UNUSED_BIT)) | (DWORD64)copy_top->p_next);

			// 스택에 변화가 있었다면 다시시도
			if (copy_unique_top != InterlockedCompareExchange64((LONG64*)&unique_top, (LONG64)unique_next, (LONG64)copy_unique_top))
				continue;

			InterlockedDecrement((LONG*)&node_count);
			*dst = copy_top->data;
			node_pool.Free(copy_top);
			return true;
		}
		// empty!!
		else {
			return false;
		}
	}
}

template <typename T>
int LFStack<T>::GetUseCount() {
	return node_count;
}

//------------------------------
// Node
//------------------------------

template <typename T>
LFStack<T>::Node::Node() : p_next(nullptr) {
}

template <typename T>
LFStack<T>::Node::Node(const T& data) : data(data), p_next(nullptr) {
}

template <typename T>
LFStack<T>::Node::~Node() {
}

template<typename T>
void LFStack<T>::Node::Clear() {
	p_next = nullptr;
}