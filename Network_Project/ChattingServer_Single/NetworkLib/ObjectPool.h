#pragma once

#include <stdarg.h>
#include <memory>
#include <BaseTsd.h>

#ifdef _WIN64
#define MEM_GUARD 0xFDFDFDFDFDFDFDFD

#else
#define MEM_GUARD 0xFDFDFDFD

#endif

namespace J_LIB {

template <typename T>
struct Pool_Node;

template <typename T>
class ObjectPool {
public:
	// node_num 만큼 node 동적할당, flag set (true : Alloc 시 마다 생성자 call, false : 노드 생성 시 최초 1회 생성자 call)
	ObjectPool(int node_num = 0, bool flag_placementNew = false);

	// 스택 보유 node delete (스택이 보유하지 않는 node는 delete 불가)
	~ObjectPool();

private:
	const ULONG_PTR integrity = (ULONG_PTR)this;
	bool flag_placementNew = false;

	Pool_Node<T>* top = nullptr;
	int capacity = 0;
	int use_count = 0;

public:
	T* Alloc();

	void Free(T* p_obejct);

	inline int GetCapacityCount() const { return capacity; }
	inline int GetUseCount() const { return use_count; }
};


//------------------------------
// ObjectPool
//------------------------------

// 생성자
template<typename T>
ObjectPool<T>::ObjectPool(int node_num, bool flag_placementNew):
	integrity((ULONG_PTR)this),
	flag_placementNew(flag_placementNew),

	top(nullptr),
	capacity(node_num),
	use_count(0)
{
	for (int i = 0; i < node_num; i++) {
		Pool_Node<T>* new_node = new Pool_Node<T>((ULONG_PTR)this);
		new_node->next_node = top;
		top = new_node;
	}
}

// 소멸자
template<typename T>
ObjectPool<T>::~ObjectPool(){
	for (; top != nullptr;) {
		Pool_Node<T>* delete_node = top;
		top = top->next_node;
		delete delete_node;
	}
}

template<typename T>
T* ObjectPool<T>::Alloc() {
	if (top) {
		if (flag_placementNew) {
			new (&top->obejct) T;
		}

		auto old_top = top;
		top = top->next_node;
		++use_count;

		return &old_top->obejct;
	}
	// 오브젝트 풀 비었을때
	else {
		++capacity;
		++use_count;

		// Node 중 Object 포인터 ret
		return &(new Pool_Node<T>((ULONG_PTR)this))->obejct;
	}
}

template<typename T>
inline void ObjectPool<T>::Free(T* p_obejct) {
	Pool_Node<T>* node = (Pool_Node<T>*)((char*)p_obejct - sizeof(size_t) - sizeof(ULONG_PTR));

	if (integrity != node->integrity)
		throw;
	if (MEM_GUARD != node->over)
		throw;
	if (MEM_GUARD != node->under)
		throw;

	if (flag_placementNew) {
		p_obejct->~T();
	}

	--use_count;
	node->next_node = top;
	top = node;
}

//------------------------------
// Node
//------------------------------

template <typename T>
struct Pool_Node {
public:
	Pool_Node(ULONG_PTR integrity) : integrity(integrity), under(MEM_GUARD), obejct(), over(MEM_GUARD), next_node(nullptr) {};

	// [코드][언더] [ OBJECT ] [오버] [ NEXT* ]
public:
	const ULONG_PTR integrity;
	const size_t under = MEM_GUARD;
	T obejct;
	const size_t over = MEM_GUARD;
	Pool_Node* next_node = nullptr;
};

}
