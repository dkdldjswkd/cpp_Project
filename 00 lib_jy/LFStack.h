#pragma once
#include <Windows.h>
#include "LFObjectPool.h"

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
	LFObjectPool<Node> nodePool;

private:
	alignas(64) DWORD64 top_ABA = NULL;
	alignas(64) int nodeCount = 0;

public:
	void Push(T data);
	bool Pop(T* dst);
	int GetUseCount();
};

//------------------------------
// LFStack
//------------------------------

template <typename T>
LFStack<T>::LFStack() : nodePool(0, true) {
}

template <typename T>
LFStack<T>::~LFStack() {
	Node* top = (Node*)(top_ABA & useBitMask);

	for (; top != nullptr;) {
		Node* delete_node = top;
		top = top->next;
		nodePool.Free(delete_node);
	}
}

template <typename T>
void LFStack<T>::Push(T data) {
	// Create Node
	Node* insert_node = nodePool.Alloc();
	insert_node->Clear();
	insert_node->data = data;

	for (;;) {
		DWORD64 copy_ABA = top_ABA;

		// top에 이어줌
		insert_node->next = (Node*)(copy_ABA & useBitMask);

		// aba count 추출 및 new_ABA 생성
		DWORD64 aba_count = (copy_ABA + stampCount) & stampMask;
		Node* new_ABA = (Node*)(aba_count | (DWORD64)insert_node);

		// 스택에 변화가 있었다면 다시시도
		if ((DWORD64)copy_ABA != InterlockedCompareExchange64((LONG64*)&top_ABA, (LONG64)new_ABA, (LONG64)copy_ABA))
			continue;

		InterlockedIncrement((LONG*)&nodeCount);
		return;
	}
}

template <typename T>
bool LFStack<T>::Pop(T* dst) {
	for (;;) {
		DWORD64 copy_ABA = top_ABA;
		Node* copy_top = (Node*)(copy_ABA & useBitMask);

		// Not empty!!
		if (copy_top) {
			// aba count 추출 및 new_ABA 생성
			DWORD64 aba_count = (copy_ABA + stampCount) & stampMask;
			Node* new_ABA = (Node*)(aba_count | (DWORD64)copy_top->next);

			// 스택에 변화가 있었다면 다시시도
			if (copy_ABA != InterlockedCompareExchange64((LONG64*)&top_ABA, (LONG64)new_ABA, (LONG64)copy_ABA))
				continue;

			InterlockedDecrement((LONG*)&nodeCount);
			*dst = copy_top->data;
			nodePool.Free(copy_top);
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
	return nodeCount;
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