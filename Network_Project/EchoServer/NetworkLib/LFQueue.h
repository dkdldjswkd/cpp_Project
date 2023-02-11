#pragma once
#include <Windows.h>
#include "LFObjectPool.h"

// 오전 4:18 2023-01-09
//
// * 해당 LFQueue 구현 환경 : Release, x64, 최적화 컴파일 OFF
//		J_LIB::LFObjectPool에 종속적
// 사본

template <typename T>
class LFQueue {
private:
	struct Node {
	public:
		Node();
		Node(const T& data);
		~Node();

	public:
		T data;
		alignas(64) Node* next;
	};

public:
	LFQueue();
	~LFQueue();

private:
	J_LIB::LFObjectPool<Node> node_pool;

private:
	const DWORD64 node_mask  = 0x00007FFFFFFFFFFF; // 노드 추출 목적 (상위 17bit 제거)
	const DWORD64 stamp_mask = 0xFFFF800000000000; // 노드 부 제거 (스탬프 추출 목적)

public:
	alignas(64) DWORD64 head_ABA;
	alignas(64) DWORD64 tail_ABA;
	alignas(32) int size = 0;

public:
	void Enqueue(T data);
	bool Dequeue(T* data);
	int  GetUseCount();
};

//------------------------------
// LFQueue
//------------------------------

template<typename T>
LFQueue<T>::LFQueue() : node_pool(0, true) {
	head_ABA = (DWORD64)node_pool.Alloc();
	((Node*)head_ABA)->next = NULL;
	tail_ABA = head_ABA;
}

template<typename T>
LFQueue<T>::~LFQueue() {
	Node* head = (Node*)(head_ABA & node_mask);

	for (; head != nullptr;) {
		Node* delete_node = head;
		head = head->next;
		node_pool.Free(delete_node);
	}
}

template<typename T>
void LFQueue<T>::Enqueue(T data) {
	Node* insert_node = node_pool.Alloc();
	insert_node->data = data;

	for (;;) {
		DWORD64 copy_tail_ABA = tail_ABA;
		Node* copy_tail = (Node*)(copy_tail_ABA & node_mask);
		Node* copy_tail_next = copy_tail->next;

		if (copy_tail_next == nullptr) {
			// Enqueue 시도
			if (InterlockedCompareExchange64((LONG64*)&copy_tail->next, (LONG64)insert_node, NULL) == NULL) {
				InterlockedIncrement((LONG*)&size);

				// tail 이동
				DWORD64 new_tail_ABA = ((copy_tail_ABA + UNUSED_COUNT) & stamp_mask) | (DWORD64)insert_node;
				LONG64 ret = InterlockedCompareExchange64((LONG64*)&tail_ABA, (LONG64)new_tail_ABA, (LONG64)copy_tail_ABA);
				return;
			}
		}
		else {
			// tail 이동
			DWORD64 next_tail_ABA = ((copy_tail_ABA + UNUSED_COUNT) & stamp_mask) | (DWORD64)copy_tail_next;
			LONG64 ret = InterlockedCompareExchange64((LONG64*)&tail_ABA, (LONG64)next_tail_ABA, (LONG64)copy_tail_ABA);
		}
	}
}

template<typename T>
bool LFQueue<T>::Dequeue(T* data) {
	if (InterlockedDecrement((LONG*)&size) < 0) {
		InterlockedIncrement((LONG*)&size);
		return false;
	}

	for (int i = 0;; i++) {
		DWORD64 copy_head_ABA = head_ABA;
		DWORD64 copy_tail_ABA = tail_ABA;

		//------------------------------
		// head, tail 역전 방지
		//------------------------------
		if (copy_head_ABA == copy_tail_ABA) {
			Node* copy_tail = (Node*)(copy_tail_ABA & node_mask);
			Node* copy_tail_next = copy_tail->next;

			// ABA or 큐에 Eq 반영 안됨
			if (copy_tail_next == nullptr)
				continue;

			// tail 밀기
			DWORD64 next_tail_ABA = ((copy_tail_ABA + UNUSED_COUNT) & stamp_mask) | (DWORD64)copy_tail_next;
			auto ret = InterlockedCompareExchange64((LONG64*)&tail_ABA, (LONG64)next_tail_ABA, (LONG64)copy_tail_ABA);
		}

		//------------------------------
		// Dequeue
		//------------------------------
		Node* copy_head = (Node*)(copy_head_ABA & node_mask);
		Node* copy_head_next = copy_head->next;

		// ABA
		if (copy_head_next == nullptr)
			continue;

		T dq_data = copy_head_next->data;

		// Dequeue 시도
		DWORD64 next_head_ABA = ((copy_head_ABA + UNUSED_COUNT) & stamp_mask) | (DWORD64)copy_head_next;
		if (InterlockedCompareExchange64((LONG64*)&head_ABA, (LONG64)next_head_ABA, (LONG64)copy_head_ABA) == (DWORD64)copy_head_ABA) {
			*data = dq_data;
			node_pool.Free(copy_head);
			return true;
		}
	}
}

template<typename T>
int LFQueue<T>::GetUseCount() {
	return size;
}

//------------------------------
// Node
//------------------------------

template <typename T>
LFQueue<T>::Node::Node() : next(nullptr) {
}

template <typename T>
LFQueue<T>::Node::Node(const T& data) : data(data), next(nullptr) {
}

template <typename T>
LFQueue<T>::Node::~Node() {
}