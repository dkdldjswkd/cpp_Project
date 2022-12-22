#pragma once
#include <Windows.h>
#include "LFObjectPool.h"
#include "MemoryLog.h"

// * �ش� LFQueue ���� ȯ�� : Release, x64, ����ȭ ������ OFF
//		J_LIB::LFObjectPool�� ������
// �纻


#define THREAD_NUM	2
#define TEST_LOOP	100
#define PUSH_LOOP	2
#define MAX_NODE	THREAD_NUM * PUSH_LOOP

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
		__declspec(align(64)) Node* next;

	public:
		void Clear();
	};

public:
	LFQueue();
	~LFQueue();

private:
	J_LIB::LFObjectPool<Node> node_pool;

private:
	const DWORD64 mask			= 0x00007FFFFFFFFFFF; // ���� 17bit 0, aba ������ ����� ���� ���
	const DWORD64 mask_stamp	= 0xFFFF800000000000; // ���� 17bit 0, aba ������ �����ϱ� ���� ���

public:
	__declspec(align(64)) DWORD64 head_ABA;
	__declspec(align(64)) DWORD64 tail_ABA;
	__declspec(align(32)) int size = 0;

public:
	void Enqueue(T data, BYTE thread_id);
	bool Dequeue(T* data, BYTE thread_id);
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
LFQueue<T>::~LFQueue(){
	// �����
	int count = 0;

	Node* head = (Node*)(head_ABA & mask);

	for (; head != nullptr;) {
		Node* delete_node = head;
		head = head->next;
		node_pool.Free(delete_node);

		// �����
		count++;
	}

	printf("remain node count : %d / delete node count : %d \n", size, count);
}

template<typename T>
void LFQueue<T>::Enqueue(T data, BYTE thread_id) {
	char* wp;

	Node* insert_node = node_pool.Alloc();
	insert_node->data = data;
	wp = memLogger.Log(thread_id + 0, &insert_node);

	for (;;) {
		DWORD64 copy_tail_ABA = tail_ABA;
		Node* copy_tail = (Node*)(copy_tail_ABA & mask);
		wp = memLogger.Log(thread_id + 1, &copy_tail);

		Node* copy_tail_next = copy_tail->next;
		wp = memLogger.Log(thread_id + 2, &copy_tail_next);

		if (copy_tail_next == nullptr){
			// Enqueue �õ�
			if (InterlockedCompareExchange64((LONG64*)&copy_tail->next, (LONG64)insert_node, NULL) == NULL) {
				wp = memLogger.Log(thread_id + 3, &insert_node);
				int eq_size = InterlockedIncrement((LONG*)&size);
				wp = memLogger.Log(thread_id + 3, &eq_size);

				// new_ABA ����
				DWORD64 tail_aba_count = (copy_tail_ABA + UNUSED_COUNT) & mask_stamp;
				DWORD64 new_tail_ABA = (tail_aba_count | (DWORD64)insert_node);
				LONG64 ret = InterlockedCompareExchange64((LONG64*)&tail_ABA, (LONG64)new_tail_ABA, (LONG64)copy_tail_ABA);

				if(ret == copy_tail_ABA)
					wp = memLogger.Log(thread_id + 4, &insert_node);

				return;
			}
		}
		else {
			// next_ABA ����
			DWORD64 tail_aba_count = (copy_tail_ABA + UNUSED_COUNT) & mask_stamp;
			DWORD64 next_tail_ABA = (tail_aba_count | (DWORD64)copy_tail_next);
			LONG64 ret = InterlockedCompareExchange64((LONG64*)&tail_ABA, (LONG64)next_tail_ABA, (LONG64)copy_tail_ABA);
			if (ret == copy_tail_ABA)
				wp = memLogger.Log(thread_id + 5, &copy_tail_next);
		}
	}
}

template<typename T>
bool LFQueue<T>::Dequeue(T* data, BYTE thread_id) {
	char* wp;

	int dq_size = InterlockedDecrement((LONG*)&size);
	// ���� ���� ���̳ʽ��� �ǹ����ٸ�
	if (dq_size < 0) {
		// ���� ��Ű�� ��ȯ
		InterlockedIncrement((LONG*)&size);
		return false;
	}

	for (int i=0;;i++) {
		// ���� ���� �Ǿ��ϴ� ��� ����
		wp = memLogger.Log(thread_id + 6, &dq_size);

		DWORD64 copy_head_ABA = head_ABA;
		Node* copy_head = (Node*)(copy_head_ABA & mask);
		wp = memLogger.Log(thread_id + 7, &copy_head);

		Node* copy_head_next = copy_head->next;
		wp = memLogger.Log(thread_id + 8, &copy_head_next);

		//// ABA �߻� (copy_head�� ���ο� node�� ����Ű�� ��Ȳ)
		//if (copy_head_next == nullptr) {
		//	if (i != 0 && i % 10000 == 0) printf("dq : %d, thread_id : %x \n", i, thread_id);
		//	continue;
		//}

		//------------------------------
		// head, tail ���� ����
		//------------------------------

		DWORD64 copy_tail_ABA = tail_ABA;
		if (copy_head_ABA == copy_tail_ABA) {
			Node* copy_tail = (Node*)(copy_tail_ABA & mask);
			wp = memLogger.Log(thread_id + 9, &copy_tail);

			Node* copy_tail_next = copy_tail->next;
			wp = memLogger.Log(thread_id + 10, &copy_tail_next);

			if (copy_tail_next == nullptr)
				continue;

			DWORD64 tail_aba_count = (copy_tail_ABA + UNUSED_COUNT) & mask_stamp;
			DWORD64 next_tail_ABA = (tail_aba_count | (DWORD64)copy_tail_next);
			auto ret = InterlockedCompareExchange64((LONG64*)&tail_ABA, (LONG64)next_tail_ABA, (LONG64)copy_tail_ABA);

			// tail �б� ����
			if (ret == copy_tail_ABA) {
				wp = memLogger.Log(thread_id + 11, &copy_tail_next);
				// �ٵ� tail�� nullptr�� �и�
				if (copy_tail_next == nullptr)
				CRASH();
			}

			if (i != 0 && i % 10000 == 0) printf("dq : %d, thread_id : %x \n", i, thread_id);
			continue;
		}

		//------------------------------
		// Dequeue
		//------------------------------

		// aba count ���� �� new_ABA ����
		DWORD64 haed_aba_count = (copy_head_ABA + UNUSED_COUNT) & mask_stamp;
		Node* next_head_ABA = (Node*)(haed_aba_count | (DWORD64)copy_head_next);

		// Dequeue �õ�
		if (InterlockedCompareExchange64((LONG64*)&head_ABA, (LONG64)next_head_ABA, (LONG64)copy_head_ABA) == (DWORD64)copy_head_ABA) {
			wp = memLogger.Log(thread_id + 12, &copy_head_next);

			if (copy_head_next == nullptr)
				CRASH();

			*data = copy_head_next->data;
			node_pool.Free(copy_head);
			return true;
		}
	}
}

template<typename T>
int LFQueue<T>::GetUseCount(){
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

template<typename T>
void LFQueue<T>::Node::Clear() {
	next = nullptr;
}