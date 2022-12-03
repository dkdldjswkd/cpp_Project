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

		// ���� ������
		iterator(const iterator& other)
		{
			_node = other._node;
		}

		// ���� ������
		void operator =(const iterator& other)
		{
			_node = other._node;
		}

		// ����Ʈ ����
		iterator(Node* node = nullptr)
		{
			_node = node;
		}

		iterator operator ++(int) // ����
		{
			iterator tmp(*this);

			_node = _node->_Next;
			return tmp;
		}

		// ����
		iterator& operator++() // ����
		{
			_node = _node->_Next;
			return *this;
		}

		iterator operator --(int) // ����
		{
			iterator tmp(*this);

			_node = _node->_Prev;
			return tmp;
		}

		iterator& operator--() // ����
		{
			_node = _node->_Prev;
			return *this;
		}

		T& operator *()
		{
			return _node->_data;
		}

		// ����
		bool operator ==(const iterator& other)
		{
			return _node == other._node;
		}

		// ����
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

	// ����?
	void pop_front() {
		if (empty()) { return; }

		Node* delete_node = _head._Next;
		delete_node->_Next->_Prev = &_head;
		_head._Next = delete_node->_Next;

		delete(delete_node);
		_size--;
	}

	// ����?
	void pop_back() {
		if (empty()) { return; }

		Node* delete_node = _tail._Prev;
		delete_node->_Prev->_Next = &_tail;
		_tail._Prev = delete_node->_Prev;

		delete(delete_node);
		_size--;
	}

	// ����?
	void clear() {
		while (_head._Next != &_tail) {
			pop_front();
		}
	}

	int size() { return _size; };

	bool empty() {
		return _head._Next == &_tail;
	}

	//- ���ͷ������� �� ��带 ����.
	//- �׸��� ���� ����� ���� ��带 ī��Ű�� ���ͷ����� ����
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