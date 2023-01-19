#pragma once
#include <Windows.h>
#include "LFObjectPool.h"
// LFStack - 원본

// * 해당 LFStack 구현 환경 : Release, x64, 최적화 컴파일 OFF
//	오전 3:03 2022-12-18

// J_LIB::LFObjectPool에 종속적임

#define CRASH()			do{						\
							int* p = nullptr;	\
							int a = *p;			\
						}while(false)

#define IF_CRASH(X)		do{							\
							if(X){					\
								int* p = nullptr;	\
								int a = *p;			\
							}						\
						}while(false)

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
		Node* next;

	public:
		void Clear();
	};

public:
	LFStack();
	~LFStack();

private:
	J_LIB::LFObjectPool<Node> node_pool;

private:
	__declspec(align(64)) DWORD64 top_ABA = NULL;
	__declspec(align(64)) int node_count = 0;

private:
	DWORD64 mask			= 0x00007FFFFFFFFFFF; // 상위 17bit 0
	DWORD64 mask_reverse	= 0xFFFF800000000000; // 상위 17bit 0

public:
	void Push(T data);
	bool Pop(T* dst);
	int GetUseCount();
};

//------------------------------
// LFStack
//------------------------------

template <typename T>
LFStack<T>::LFStack() : node_pool(0, true) {
}

template <typename T>
LFStack<T>::~LFStack() {
	Node* top = (Node*)(top_ABA & mask);

	for (; top != nullptr;) {
		Node* delete_node = top;
		top = top->next;
		delete delete_node;
	}
}

template <typename T>
void LFStack<T>::Push(T data) {
	// Create Node
	Node* insert_node = node_pool.Alloc();
	insert_node->Clear();
	insert_node->data = data;

	for (;;) {
		DWORD64 copy_ABA = top_ABA;

		// top에 이어줌
		insert_node->next = (Node*)(copy_ABA & mask);

		// aba count 추출 및 new_ABA 생성
		DWORD64 aba_count = (copy_ABA + UNUSED_COUNT) & mask_reverse;
		Node* new_ABA = (Node*)(aba_count | (DWORD64)insert_node);

		// 스택에 변화가 있었다면 다시시도
		if ((DWORD64)copy_ABA != InterlockedCompareExchange64((LONG64*)&top_ABA, (LONG64)new_ABA, (LONG64)copy_ABA))
			continue;

		InterlockedIncrement((LONG*)&node_count);
		return;
	}
}

template <typename T>
bool LFStack<T>::Pop(T* dst) {
	for (;;) {
		DWORD64 copy_ABA = top_ABA;
		Node* copy_top = (Node*)(copy_ABA & mask);

		// Not empty!!
		if (copy_top) {
			// aba count 추출 및 new_ABA 생성
			DWORD64 aba_count = (copy_ABA + UNUSED_COUNT) & mask_reverse;
			Node* new_ABA = (Node*)(aba_count | (DWORD64)copy_top->next);

			// 스택에 변화가 있었다면 다시시도
			if (copy_ABA != InterlockedCompareExchange64((LONG64*)&top_ABA, (LONG64)new_ABA, (LONG64)copy_ABA))
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
LFStack<T>::Node::Node() : next(nullptr) {
}

template <typename T>
LFStack<T>::Node::Node(const T& data) : data(data), next(nullptr) {
}

template <typename T>
LFStack<T>::Node::~Node() {
}

template<typename T>
void LFStack<T>::Node::Clear() {
	next = nullptr;
}