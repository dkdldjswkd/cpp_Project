#pragma once

template <typename T>
class j_list
{
public:
	struct Node
	{
		Node(T data, Node* prev, Node* next) {
			_data = data;
			_Prev = prev;
			_Next = next;
		}
		Node() {}

		T _data;
		Node* _Prev;
		Node* _Next;
	};

	class iterator
	{
	private:
		Node* _node;
		friend j_list;

	public:

		// 복사 생성자
		iterator(const iterator& other)
		{
			_node = other._node;
		}

		// 대입 연산자
		void operator =(const iterator& other)
		{
			_node = other._node;
		}

		// 디폴트 생성
		iterator(Node* node = nullptr)
		{
			_node = node;
		}

		iterator operator ++(int) // 후위
		{
			iterator tmp(*this);

			_node = _node->_Next;
			return tmp;
		}

		// 안전
		iterator& operator++() // 전위
		{
			_node = _node->_Next;
			return *this;
		}

		iterator operator --(int) // 후위
		{
			iterator tmp(*this);

			_node = _node->_Prev;
			return tmp;
		}

		iterator& operator--() // 전위
		{
			_node = _node->_Prev;
			return *this;
		}

		T& operator *()
		{
			return _node->_data;
		}

		// 안전
		bool operator ==(const iterator& other)
		{
			return _node == other._node;
		}

		// 안전
		bool operator !=(const iterator& other)
		{
			return _node != other._node;
		}
	};

public:
	j_list() :
		_head(NULL, NULL, &_tail),
		_tail(NULL, &_head, NULL)
	{
	}

	~j_list() {}

	iterator begin()
	{
		iterator tmp;
		tmp._node = _head._Next;
		return tmp;
	}

	iterator end()
	{
		iterator tmp;
		tmp._node = &_tail;
		return tmp;
	}

	void push_front(T data) {
		Node* new_node = new Node(data, &_head, _head._Next);

		new_node->_Prev->_Next = new_node;
		new_node->_Next->_Prev = new_node;

		_size++;
	}

	void push_back(T data) {
		Node* new_node = new Node(data, _tail._Prev, &_tail);

		new_node->_Prev->_Next = new_node;
		new_node->_Next->_Prev = new_node;

		_size++;
	}

	// 안전?
	void pop_front() {
		if (empty()) { return; }

		Node* delete_node = _head._Next;
		delete_node->_Next->_Prev = &_head;
		_head._Next = delete_node->_Next;

		delete(delete_node);
		_size--;
	}

	// 안전?
	void pop_back() {
		if (empty()) { return; }

		Node* delete_node = _tail._Prev;
		delete_node->_Prev->_Next = &_tail;
		_tail._Prev = delete_node->_Prev;

		delete(delete_node);
		_size--;
	}

	// 안전?
	void clear() {
		while (_head._Next != &_tail) {
			pop_front();
		}
	}

	int size() { return _size; };

	bool empty() {
		return _head._Next == &_tail;
	}

	//- 이터레이터의 그 노드를 지움.
	//- 그리고 지운 노드의 다음 노드를 카리키는 이터레이터 리턴
	iterator erase(iterator iter) {
		_size--;

		Node* delete_node = iter._node;
		delete_node->_Prev->_Next = delete_node->_Next;
		delete_node->_Next->_Prev = delete_node->_Prev;
		iter++;

		// ?
		delete(delete_node);
		return iter;
	}

	void remove(T Data)
	{
		j_list<T>::iterator iter;
		for (iter = j_list.begin(); iter != j_list.end();)
		{
			if (*iter == Data)
				erase(iter++);
			else
				iter++;
		}
	}

private:
	int _size = 0;
	Node _head;
	Node _tail;
};