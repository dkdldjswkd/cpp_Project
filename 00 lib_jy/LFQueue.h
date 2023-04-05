#pragma once
#include <Windows.h>
#include "LFObjectPool.h"

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
	LFObjectPool<Node> nodePool;

public:
	alignas(64) DWORD64 headStamp;
	alignas(64) DWORD64 tailStamp;
	alignas(64) int nodeCount = 0;

public:
	void Enqueue(T data);
	bool Dequeue(T* data);
	int  GetUseCount();
};

//------------------------------
// LFQueue
//------------------------------

template<typename T>
LFQueue<T>::LFQueue() : nodePool(0, true) {
	headStamp = (DWORD64)nodePool.Alloc();
	((Node*)headStamp)->next = NULL;
	tailStamp = headStamp;
}

template<typename T>
LFQueue<T>::~LFQueue() {
	Node* head = (Node*)(headStamp & useBitMask);

	for (; head != nullptr;) {
		Node* deleteNode = head;
		head = head->next;
		nodePool.Free(deleteNode);
	}
}

template<typename T>
void LFQueue<T>::Enqueue(T data) {
	Node* enqueNode = nodePool.Alloc();
	enqueNode->data = data;

	for (;;) {
		DWORD64 copyTailStamp = tailStamp;
		Node* tailClean = (Node*)(copyTailStamp & useBitMask);
		Node* tailNext = tailClean->next;

		if (tailNext == nullptr) {
			// Enqueue 시도
			if (InterlockedCompareExchange64((LONG64*)&tailClean->next, (LONG64)enqueNode, NULL) == NULL) {
				InterlockedIncrement((LONG*)&nodeCount);

				// tail 이동
				DWORD64 newTailStamp = ((copyTailStamp + stampCount) & stampMask) | (DWORD64)enqueNode;
				LONG64 ret = InterlockedCompareExchange64((LONG64*)&tailStamp, (LONG64)newTailStamp, (LONG64)copyTailStamp);
				return;
			}
		}
		else {
			// tail 이동
			DWORD64 newTailStamp = ((copyTailStamp + stampCount) & stampMask) | (DWORD64)tailNext;
			InterlockedCompareExchange64((LONG64*)&tailStamp, (LONG64)newTailStamp, (LONG64)copyTailStamp);
		}
	}
}

template<typename T>
bool LFQueue<T>::Dequeue(T* data) {
	if (InterlockedDecrement((LONG*)&nodeCount) < 0) {
		InterlockedIncrement((LONG*)&nodeCount);
		return false;
	}

	for (;;) {
		DWORD64 copyHeadStamp = headStamp;
		DWORD64 copyTailStamp = tailStamp;

		//------------------------------
		// head, tail 역전 방지
		//------------------------------
		if (copyHeadStamp == copyTailStamp) {
			Node* tailClean = (Node*)(copyTailStamp & useBitMask);
			Node* tailNext = tailClean->next;

			// ABA or 큐에 Eq 반영 안됨 headNext
			if (tailNext == nullptr)
				continue;

			// tail 밀기
			DWORD64 newTailStamp = ((copyTailStamp + stampCount) & stampMask) | (DWORD64)tailNext;
			auto ret = InterlockedCompareExchange64((LONG64*)&tailStamp, (LONG64)newTailStamp, (LONG64)copyTailStamp);
		}

		//------------------------------
		// Dequeue
		//------------------------------
		Node* headClean = (Node*)(copyHeadStamp & useBitMask);
		Node* headNext = headClean->next;

		// ABA
		if (headNext == nullptr)
			continue;

		T dqData = headNext->data;

		// Dequeue 시도
		DWORD64 newHeadStamp = ((copyHeadStamp + stampCount) & stampMask) | (DWORD64)headNext;
		if (InterlockedCompareExchange64((LONG64*)&headStamp, (LONG64)newHeadStamp, (LONG64)copyHeadStamp) == (DWORD64)copyHeadStamp) {
			*data = dqData;
			nodePool.Free(headClean);
			return true;
		}
	}
}

template<typename T>
int LFQueue<T>::GetUseCount() {
	return nodeCount;
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