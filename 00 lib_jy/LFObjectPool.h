#pragma once
#include "base.h"
#include <Windows.h>
#include <stdarg.h>
#include <memory>
#include <BaseTsd.h>

template <typename T>
class LFObjectPool {
private:
	static struct Node {
	public:
		Node(ULONG_PTR integrity) : integrity(integrity), under_guard(memGuard), obejct(), over_guard(memGuard), next(nullptr) {};

	public:
		const ULONG_PTR integrity;
		const size_t under_guard;
		T obejct;
		const size_t over_guard;
		Node* next = nullptr;
	};

public:
	LFObjectPool(int node_num = 0, bool use_ctor = false);
	~LFObjectPool();

private:
	const ULONG_PTR integrity;
	alignas(64) ULONG_PTR topStamp;
	int object_offset;

	// Opt
	bool use_ctor;

	// Count
	alignas(64) int capacity;
	alignas(64) int use_count;

public:
	T* Alloc();
	void Free(T* p_obejct);

	// Opt
	void SetUseCtor(bool isUse) { use_ctor = isUse; }

	// Getter
	inline int GetCapacityCount() const { return capacity; }
	inline int GetUseCount() const { return use_count; }
};

//------------------------------
// ObjectPool
//------------------------------

template<typename T>
LFObjectPool<T>::LFObjectPool(int node_num, bool use_ctor) : integrity((ULONG_PTR)this), use_ctor(use_ctor), topStamp(NULL), capacity(node_num), use_count(0) {
	// Stamp ��� ���� ���� Ȯ��
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	if (0 != ((ULONG_PTR)sys_info.lpMaximumApplicationAddress & stampMask))
		throw std::exception("STAMP_N/A");

	// object_offset �ʱ�ȭ
	Node tmpNode((ULONG_PTR)this);
	object_offset = ((ULONG_PTR)(&tmpNode.obejct) - (ULONG_PTR)&tmpNode);

	// ����� ��û ��� ����
	for (int i = 0; i < node_num; i++) {
		Node* new_node = new Node((ULONG_PTR)this);
		new_node->next = (Node*)topStamp;
		topStamp = (ULONG_PTR)new_node;
	}
}

template<typename T>
LFObjectPool<T>::~LFObjectPool() {
	Node* top = (Node*)(topStamp & useBitMask);

	for (; top != nullptr;) {
		Node* delete_node = top;
		top = top->next;
		delete delete_node;
	}
}

// ������ ������ POP�� �ش�
template<typename T>
T* LFObjectPool<T>::Alloc() {
	for (;;) {
		ULONG_PTR copyTopStamp = topStamp;
		Node* topClean = (Node*)(copyTopStamp & useBitMask);

		// Not empty!!
		if (topClean) {
			// aba count ���� �� newTopStamp ����
			ULONG_PTR nextStamp = (copyTopStamp + stampCount) & stampMask;
			Node* newTopStamp = (Node*)(nextStamp | (ULONG_PTR)topClean->next);

			// ���ÿ� ��ȭ�� �־��ٸ� �ٽýõ�
			if (copyTopStamp != InterlockedCompareExchange64((LONG64*)&topStamp, (LONG64)newTopStamp, (LONG64)copyTopStamp))
				continue;

			if (use_ctor) {
				new (&topClean->obejct) T;
			}

			InterlockedIncrement((LONG*)&use_count);
			return &topClean->obejct;
		}
		// empty!!
		else {
			InterlockedIncrement((LONG*)&capacity);
			InterlockedIncrement((LONG*)&use_count);

			// Node �� Object ������ ret
			return &(new Node((ULONG_PTR)this))->obejct;
		}
	}
}

// ������ ������ PUSH�� �ش�
template<typename T>
void LFObjectPool<T>::Free(T* p_obejct) {
	// ������Ʈ ���� ��ȯ
	Node* node = (Node*)((char*)p_obejct - object_offset);

	if (integrity != node->integrity)
		throw std::exception("ERROR_INTEGRITY");
	if (memGuard != node->over_guard)
		throw std::exception("ERROR_INVAID_OVER");
	if (memGuard != node->under_guard)
		throw std::exception("ERROR_INVAID_UNDER");

	if (use_ctor) {
		p_obejct->~T();
	}

	for (;;) {
		DWORD64 copyTopStamp = topStamp;

		// top�� �̾���
		node->next = (Node*)(copyTopStamp & useBitMask);

		// aba count ���� �� newTopStamp ����
		DWORD64 newStamp = (copyTopStamp + stampCount) & stampMask;
		Node* newTopStamp = (Node*)(newStamp | (DWORD64)node);

		// ���ÿ� ��ȭ�� �־��ٸ� �ٽýõ�
		if ((DWORD64)copyTopStamp != InterlockedCompareExchange64((LONG64*)&topStamp, (LONG64)newTopStamp, (LONG64)copyTopStamp))
			continue;

		InterlockedDecrement((LONG*)&use_count);
		return;
	}
}