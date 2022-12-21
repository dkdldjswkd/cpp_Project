#pragma once
#include <Windows.h>
#include "LFObjectPool.h"
#include "MemoryLog.h"

// * 해당 LFQueue 구현 환경 : Release, x64, 최적화 컴파일 OFF
//		J_LIB::LFObjectPool에 종속적
// 사본


#define THREAD_NUM	2
#define TEST_LOOP	100
#define PUSH_LOOP	3
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
	const DWORD64 mask			= 0x00007FFFFFFFFFFF; // 상위 17bit 0, aba 스탬프 지우기 위해 사용
	const DWORD64 mask_stamp	= 0xFFFF800000000000; // 상위 17bit 0, aba 스탬프 추출하기 위해 사용

public:
	__declspec(align(64)) DWORD64 head_ABA;
	__declspec(align(64)) DWORD64 tail_ABA;
	__declspec(align(64)) int size = 0;

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
	// 디버깅
	int count = 0;

	Node* head = (Node*)(head_ABA & mask);

	for (; head != nullptr;) {
		Node* delete_node = head;
		head = head->next;
		node_pool.Free(delete_node);

		// 디버깅
		count++;
	}

	printf("remain node count : %d / delete node count : %d \n", size, count);
}

template<typename T>
void LFQueue<T>::Enqueue(T data, BYTE thread_id) {
	char* wp;

	Node* insert_node = node_pool.Alloc();
	insert_node->data = data;
	insert_node->next = NULL;

	for (;;) {
		DWORD64 copy_tail_ABA = tail_ABA;
		memLogger.Log(thread_id + 0, &copy_tail_ABA);

		Node* copy_tail = (Node*)(copy_tail_ABA & mask);
		memLogger.Log(thread_id + 1, &copy_tail);

		Node* copy_tail_next = copy_tail->next;
		memLogger.Log(thread_id + 2, &copy_tail_next);

		if (insert_node == nullptr)
			CRASH();

		if (copy_tail_next == nullptr){
			memLogger.Log(thread_id + 3);

			// Enqueue 시도
			if (InterlockedCompareExchange64((LONG64*)&copy_tail->next, (LONG64)insert_node, NULL) == NULL) {
				InterlockedIncrement((LONG*)&size);

				// new_ABA 생성
				DWORD64 tail_aba_count = (copy_tail_ABA + UNUSED_COUNT) & mask_stamp;
				DWORD64 new_tail_ABA = (tail_aba_count | (DWORD64)insert_node);
				InterlockedCompareExchange64((LONG64*)&tail_ABA, (LONG64)new_tail_ABA, (LONG64)copy_tail_ABA);
				return;
			}
		}
		else {
			// next_ABA 생성
			DWORD64 tail_aba_count = (copy_tail_ABA + UNUSED_COUNT) & mask_stamp;
			DWORD64 next_tail_ABA = (tail_aba_count | (DWORD64)copy_tail_next);
			if (InterlockedCompareExchange64((LONG64*)&tail_ABA, (LONG64)next_tail_ABA, (LONG64)copy_tail_ABA) == (LONG64)copy_tail_ABA) {
				// 디버깅
				if (copy_tail_next == NULL)
					CRASH();
				if (copy_tail_next == copy_tail)
					CRASH();
			}
		}
	}
}

template<typename T>
bool LFQueue<T>::Dequeue(T* data, BYTE thread_id) {
	if (InterlockedDecrement((LONG*)&size) < 0) {
		InterlockedIncrement((LONG*)&size);
		return false;
	}

	for (int i=0;;i++) {
		DWORD64 copy_head_ABA = head_ABA;
		Node* copy_head = (Node*)(copy_head_ABA & mask);
		Node* copy_head_next = copy_head->next;

		// ABA 발생 (copy_head가 새로운 node를 가르키는 상황)
		if (copy_head_next == nullptr) {
			if (i != 0 && i % 1000 == 0) printf("dq : %d, thread_id : %x \n", i, thread_id);
			continue;
		}

		//------------------------------
		// head, tail 역전 방지
		//------------------------------

		DWORD64 copy_tail_ABA = tail_ABA;
		Node* copy_tail = (Node*)(copy_tail_ABA & mask);
		Node* copy_tail_next = copy_tail->next;

		// head, tail 역전 방지
		if (copy_head_ABA == copy_tail_ABA) {
			//// ABA 발생 // ** 여기서 무한루프 돔
			//if (copy_tail_next == NULL) {
			//	if (i != 0 && i % 1000 == 0) printf("dq : %d, thread_id : %x \n", i, thread_id);
			//	continue;
			//}

			// aba count 추출 및 new_ABA 생성
			DWORD64 tail_aba_count = (copy_tail_ABA + UNUSED_COUNT) & mask_stamp;
			Node* next_tail_ABA = (Node*)(tail_aba_count | (DWORD64)copy_tail_next);

			if (InterlockedCompareExchange64((LONG64*)&tail_ABA, (LONG64)next_tail_ABA, (LONG64)copy_tail_ABA)) {
				if(copy_tail_next == nullptr)
					CRASH();
				if (copy_tail == copy_tail_next)
					CRASH();
			}
			continue;
		}

		//------------------------------
		// Dequeue
		//------------------------------

		// aba count 추출 및 new_ABA 생성
		DWORD64 haed_aba_count = (copy_head_ABA + UNUSED_COUNT) & mask_stamp;
		Node* next_head_ABA = (Node*)(haed_aba_count | (DWORD64)copy_head_next);

		// Dequeue 시도
		if (InterlockedCompareExchange64((LONG64*)&head_ABA, (LONG64)next_head_ABA, (LONG64)copy_head_ABA) == (DWORD64)copy_head_ABA) {
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