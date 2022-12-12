#pragma once

// * 해당 LFQueue 구현 환경 : Release, x64, 최적화 컴파일 OFF

template <typename T>
class LFQueue {
private:
	long _size;

	struct Node {
		T data;
		Node* next;
	};

	Node* head;
	Node* tail;

public:
	LFQueue();
	void Enqueue(T t);
	int Dequeue(T& t);
};

template<typename T>
LFQueue<T>::LFQueue() {
	_size = 0;
	head = new Node;
	head->next = NULL;
	tail = head;
}

template<typename T>
void LFQueue<T>::Enqueue(T t) {
	Node* node = new Node;
	node->data = t;
	node->next = NULL;

	while (true) {
		Node* tail = tail;
		Node* next = tail->next;

		if (next == NULL) {
			if (InterlockedCompareExchangePointer((PVOID*)&tail->next, node, next) == next) {
				InterlockedCompareExchangePointer((PVOID*)&tail, node, tail);
				break;
			}
		}
	}

	InterlockedExchangeAdd(&_size, 1);
}

template<typename T>
int LFQueue<T>::Dequeue(T& t) {
	if (_size == 0)
		return -1;

	while (true) {
		Node* head = head;
		Node* next = head->next;

		if (next == NULL) {
			return -1;
		}
		else {
			if (InterlockedCompareExchangePointer((PVOID*)&head, next, head) == head) {
				t = next->data;
				delete head;
				break;
			}
		}
	}
	InterlockedExchangeAdd(&_size, -1);
	return 0;
}