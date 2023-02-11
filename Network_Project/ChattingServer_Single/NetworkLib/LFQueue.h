#pragma once
#include <Windows.h>
#include "LFObjectPool.h"

// ���� 4:18 2023-01-09
//
// * �ش� LFQueue ���� ȯ�� : Release, x64, ����ȭ ������ OFF
//		J_LIB::LFObjectPool�� ������
// �纻

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
	const DWORD64 node_mask  = 0x00007FFFFFFFFFFF; // ��� ���� ���� (���� 17bit ����)
	const DWORD64 stamp_mask = 0xFFFF800000000000; // ��� �� ���� (������ ���� ����)

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
			// Enqueue �õ�
			if (InterlockedCompareExchange64((LONG64*)&copy_tail->next, (LONG64)insert_node, NULL) == NULL) {
				InterlockedIncrement((LONG*)&size);

				// tail �̵�
				DWORD64 new_tail_ABA = ((copy_tail_ABA + UNUSED_COUNT) & stamp_mask) | (DWORD64)insert_node;
				LONG64 ret = InterlockedCompareExchange64((LONG64*)&tail_ABA, (LONG64)new_tail_ABA, (LONG64)copy_tail_ABA);
				return;
			}
		}
		else {
			// tail �̵�
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
		// head, tail ���� ����
		//------------------------------
		if (copy_head_ABA == copy_tail_ABA) {
			Node* copy_tail = (Node*)(copy_tail_ABA & node_mask);
			Node* copy_tail_next = copy_tail->next;

			// ABA or ť�� Eq �ݿ� �ȵ�
			if (copy_tail_next == nullptr)
				continue;

			// tail �б�
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

		// Dequeue �õ�
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