#pragma once
#include <Windows.h>
#include "LFObjectPool.h"

// * �ش� LFQueue ���� ȯ�� : Release, x64, ����ȭ ������ OFF
//		J_LIB::LFObjectPool�� ������
// �纻

#include "MemoryNote.h"
extern MemoryNote note;

// ������ ���� �ڵ� ��ġ�� ����
#define LOG(X)  do{								\
					wp = note.get_wp();			\
					*wp = X + thread_id;		\
					ZeroMemory(wp + 1, 7);		\
				}while(false)

// ������ ���� �ڵ� ��ġ��
// log_data�� ���� wp + 1 ���� 7byte ����
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
	// ���߿� ����
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
			// Enqueue ����
			if (InterlockedCompareExchange64((LONG64*)&copy_tail->next, (LONG64)insert_node, NULL) == NULL) {
				// tail �ڷ� �̵�
				InterlockedCompareExchange64((LONG64*)&tail, (LONG64)insert_node, (LONG64)copy_tail);
				// size count
				InterlockedIncrement((LONG*)&size);
				return;
			}
		}
		else {
			// tail �ڷ� �̵�
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
		
		// head�� tail�� �پ���� �ʰ�
		if (copy_head == copy_tail) {
			InterlockedCompareExchange64((LONG64*)&tail, (LONG64)copy_next, (LONG64)copy_tail);
			continue;
		}

		// Dequeue ����
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