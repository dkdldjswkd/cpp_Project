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
		Node(ULONG_PTR integrity) : integrity(integrity), underGuard(memGuard), obejct(), overGuard(memGuard), next(nullptr) {};

	public:
		const ULONG_PTR integrity;
		const size_t underGuard;
		T obejct;
		const size_t overGuard;
		Node* next = nullptr;
	};

public:
	LFObjectPool(int node_num = 0, bool use_ctor = false);
	~LFObjectPool();

private:
	const ULONG_PTR integrity;
	alignas(64) ULONG_PTR topStamp;
	int objectOffset;

	// Opt
	bool useCtor;

	// Count
	alignas(64) int capacity;
	alignas(64) int useCount;

public:
	T* Alloc();
	void Free(T* p_obejct);

	// Opt
	void SetUseCtor(bool isUse) { useCtor = isUse; }

	// Getter
	inline int GetCapacityCount() const { return capacity; }
	inline int GetUseCount() const { return useCount; }
};

//------------------------------
// ObjectPool
//------------------------------

template<typename T>
LFObjectPool<T>::LFObjectPool(int node_num, bool use_ctor) : integrity((ULONG_PTR)this), useCtor(use_ctor), topStamp(NULL), capacity(node_num), useCount(0) {
	// Stamp ��� ���� ���� Ȯ��
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	if (0 != ((ULONG_PTR)sysInfo.lpMaximumApplicationAddress & stampMask))
		throw std::exception("STAMP_N/A");

	// object_offset �ʱ�ȭ
	Node tmpNode((ULONG_PTR)this);
	objectOffset = ((ULONG_PTR)(&tmpNode.obejct) - (ULONG_PTR)&tmpNode);

	// ����� ��û ��� ����
	for (int i = 0; i < node_num; i++) {
		Node* newNode = new Node((ULONG_PTR)this);
		newNode->next = (Node*)topStamp;
		topStamp = (ULONG_PTR)newNode;
	}
}

template<typename T>
LFObjectPool<T>::~LFObjectPool() {
	Node* top = (Node*)(topStamp & useBitMask);

	for (; top != nullptr;) {
		Node* deleteNode = top;
		top = top->next;
		delete deleteNode;
	}
}

// ������ ������ POP�� �ش�
template<typename T>
T* LFObjectPool<T>::Alloc() {
	for (;;) {
		DWORD64 copyTopStamp = topStamp;
		Node* topClean = (Node*)(copyTopStamp & useBitMask);

		// Not empty!!
		if (topClean) {
			// Stamp ���� �� newTopStamp ����
			DWORD64 nextStamp = (copyTopStamp + stampCount) & stampMask;
			DWORD64 newTopStamp = nextStamp | (DWORD64)topClean->next;

			// ���ÿ� ��ȭ�� �־��ٸ� �ٽýõ�
			if (copyTopStamp != InterlockedCompareExchange64((LONG64*)&topStamp, (LONG64)newTopStamp, (LONG64)copyTopStamp))
				continue;

			if (useCtor) {
				new (&topClean->obejct) T;
			}

			InterlockedIncrement((LONG*)&useCount);
			return &topClean->obejct;
		}
		// empty!!
		else {
			InterlockedIncrement((LONG*)&capacity);
			InterlockedIncrement((LONG*)&useCount);

			// Node �� Object ������ ret
			return &(new Node((ULONG_PTR)this))->obejct;
		}
	}
}

// ������ ������ PUSH�� �ش�
template<typename T>
void LFObjectPool<T>::Free(T* p_obejct) {
	// ������Ʈ ���� ��ȯ
	Node* pushNode = (Node*)((char*)p_obejct - objectOffset);

	if (integrity != pushNode->integrity)
		throw std::exception("ERROR_INTEGRITY");
	if (memGuard != pushNode->overGuard)
		throw std::exception("ERROR_INVAID_OVER");
	if (memGuard != pushNode->underGuard)
		throw std::exception("ERROR_INVAID_UNDER");

	if (useCtor) {
		p_obejct->~T();
	}

	for (;;) {
		DWORD64 copyTopStamp = topStamp;

		// top�� �̾���
		pushNode->next = (Node*)(copyTopStamp & useBitMask);

		// Stamp ���� �� newTopStamp ����
		DWORD64 nextStamp = (copyTopStamp + stampCount) & stampMask;
		DWORD64 newTopStamp = nextStamp | (DWORD64)pushNode;

		// ���ÿ� ��ȭ�� �־��ٸ� �ٽýõ�
		if (copyTopStamp != InterlockedCompareExchange64((LONG64*)&topStamp, (LONG64)newTopStamp, (LONG64)copyTopStamp))
			continue;

		InterlockedDecrement((LONG*)&useCount);
		return;
	}
}