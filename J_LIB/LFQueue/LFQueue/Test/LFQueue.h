#pragma once
#include <Windows.h>
#include "LFObjectPool.h"

// * 해당 LFQueue 구현 환경 : Release, x64, 최적화 컴파일 OFF
//		J_LIB::LFObjectPool에 종속적
// 사본

#include "MemoryNote.h"
extern MemoryNote note;

// 스레드 진행 코드 위치만 남김
#define LOG(X)  do{								\
					wp = note.get_wp();			\
					*wp = X + thread_id;		\
					ZeroMemory(wp + 1, 7);		\
				}while(false)

// 스레드 진행 코드 위치와
// log_data의 값을 wp + 1 부터 7byte 남김
#define LOG_(X, log_data)	do{									\
								wp = note.get_wp();				\
								*wp = X + thread_id;			\
								memmove(wp + 1, log_data, 7);	\
							}while(false)

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

public:
	__declspec(align(64)) Node* head;
	__declspec(align(64)) Node* tail;
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
	head = node_pool.Alloc();
	head->next = NULL;
	tail = head;
}

template<typename T>
LFQueue<T>::~LFQueue(){
	// 나중에 구현
}

template<typename T>
void LFQueue<T>::Enqueue(T data, BYTE thread_id) {
	Node* insert_node = node_pool.Alloc();
	insert_node->data = data;
	insert_node->next = NULL;

	for (;;) {
		Node* copy_tail = tail;
		Node* copy_next = copy_tail->next;

		if (copy_next == null){
			// Enqueue 성공
			if (InterlockedCompareExchange64((LONG64*)&copy_tail->next, (LONG64)insert_node, NULL) == NULL) {
				// tail 뒤로 이동
				InterlockedCompareExchange64((LONG64*)&tail, (LONG64)insert_node, (LONG64)copy_tail);
				// size count
				InterlockedIncrement((LONG*)&size);
				return;
			}
		}
		else {
			// tail 뒤로 이동
			InterlockedCompareExchange64((LONG64*)&tail, (LONG64)copy_next, (LONG64)copy_tail);
		}
	}
}

template<typename T>
bool LFQueue<T>::Dequeue(T* data, BYTE thread_id) {
	for (;;) {
		Node* copy_head = head;
		Node* copy_next = copy_head->next;
		Node* copy_tail = tail;

		// Check Empty
		if (copy_next == nullptr) return false;
		
		// head가 tail을 뛰어넘지 않게
		if (copy_head == copy_tail) {
			InterlockedCompareExchange64((LONG64*)&tail, (LONG64)copy_next, (LONG64)copy_tail);
			continue;
		}

		// Dequeue 성공
		if (InterlockedCompareExchange64((LONG64*)&head, (LONG64)copy_next, (LONG64)copy_head) == (LONG64)copy_head) {
			*data = copy_next->data;
			node_pool.Free(copy_head);
			InterlockedDecrement((LONG*)&size);
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