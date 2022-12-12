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
	Node* insert_node = new Node;
	insert_node->data = t;
	insert_node->next = NULL;

	while (true) {
		Node* copy_tail = tail;
		Node* next = copy_tail->next;

		if (next == NULL) {
			// Enqueue 성공
			if (InterlockedCompareExchangePointer((PVOID*)&tail->next, insert_node, NULL) == NULL) {
				// tail 뒤로 밀기
				InterlockedCompareExchangePointer((PVOID*)&tail, insert_node, copy_tail);
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