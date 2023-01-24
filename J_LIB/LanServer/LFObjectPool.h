#pragma once

#include <Windows.h>
#include <stdarg.h>
#include <memory>
#include <BaseTsd.h>
// LFObjectPool - ����

// * �ش� LFObjectPool ���� ȯ�� : Release, x64, ����ȭ ������ OFF
//	���� 3:14 2022-12-18

#define CRASH() do{				\
					int* p = 0;	\
					*p = 0;		\
				}while(false)

constexpr BYTE	  UNUSED_BIT	= 17;
constexpr DWORD64 UNUSED_COUNT	= 0x800000000000;

#define MEM_GUARD 0xFDFDFDFDFDFDFDFD

namespace J_LIB {
	template <typename T>
	struct Node;

	template <typename T>
	class LFObjectPool {
	private:
		struct Node {
		public:
			Node(ULONG_PTR integrity) : integrity(integrity), under(MEM_GUARD), obejct(), over(MEM_GUARD), next_node(nullptr) {};

			//   8     8        ?        8       8
			// [�ڵ�][���] [ OBJECT ] [����] [ NEXT* ]
		public:
			const ULONG_PTR integrity;
			const size_t under = MEM_GUARD;
			T obejct;
			const size_t over = MEM_GUARD;
			Node* next_node = nullptr;
		};

	public:
		// node_num ��ŭ node �����Ҵ�, flag set (true : Alloc �� ���� ������ call, false : ��� ���� �� ���� 1ȸ ������ call)
		LFObjectPool(int node_num = 0, bool flag_placementNew = false);

		// ���� ���� node delete (������ �������� �ʴ� node�� delete �Ұ�)
		~LFObjectPool();

	private:
		const ULONG_PTR integrity = (ULONG_PTR)this;
		bool flag_placementNew = false;
		__declspec(align(64)) DWORD64 top_ABA = NULL;
		__declspec(align(64)) int capacity = 0;
		__declspec(align(64)) int use_count = 0;
		int object_offset = 0;
		DWORD64 mask		 = 0x00007FFFFFFFFFFF; // ���� 17bit 0
		DWORD64 mask_reverse = 0xFFFF800000000000; // ���� 17bit 0

	public:
		T* Alloc();
		void Free(T* p_obejct);
		inline int GetCapacityCount() const { return capacity; }
		inline int Get_UseCount() const { return use_count; }
	};

	//------------------------------
	// ObjectPool
	//------------------------------

	// ������ (* ȣ�� �� ��Ƽ �����忡���� Alloc, Free ȣ���� �־ �ȵ�)
	template<typename T>
	LFObjectPool<T>::LFObjectPool(int node_num, bool flag_placementNew) :
		integrity((ULONG_PTR)this),
		flag_placementNew(flag_placementNew),

		top_ABA(NULL),
		capacity(node_num),
		use_count(0)
	{
		// object_offset�� �����ϱ� ���� Node ����
		Node* new_node = new Node((ULONG_PTR)this);
		new_node->next_node = (Node*)top_ABA;
		top_ABA = (DWORD64)new_node;
		object_offset = ((int)(&new_node->obejct) - (int)new_node);
		capacity++;

		// ��û�� ��� ������ŭ �̸� ��� ����
		for (int i = 0; i < node_num - 1; i++) {
			Node* new_node = new Node((ULONG_PTR)this);
			new_node->next_node = (Node*)top_ABA;
			top_ABA = (DWORD64)new_node;
		}
	}

	// �Ҹ��� (* ȣ�� �� ��Ƽ �����忡���� Alloc, Free ȣ���� �־ �ȵ�)
	template<typename T>
	LFObjectPool<T>::~LFObjectPool() {
		Node* top = (Node*)(top_ABA & mask);

		for (; top != nullptr;) {
			Node* delete_node = top;
			top = top->next_node;
			delete delete_node;
		}
	}

	// ������ ������ POP�� �ش�
	template<typename T>
	T* LFObjectPool<T>::Alloc() {
		for (;;) {
			DWORD64 copy_ABA = top_ABA;
			Node* copy_top = (Node*)(copy_ABA & mask);

			// Not empty!!
			if (copy_top) {
				// aba count ���� �� new_ABA ����
				DWORD64 aba_count = (copy_ABA + UNUSED_COUNT) & mask_reverse;
				Node* new_ABA = (Node*)(aba_count | (DWORD64)copy_top->next_node);

				// ���ÿ� ��ȭ�� �־��ٸ� �ٽýõ�
				if (copy_ABA != InterlockedCompareExchange64((LONG64*)&top_ABA, (LONG64)new_ABA, (LONG64)copy_ABA))
					continue;

				if (flag_placementNew) {
					new (&copy_top->obejct) T;
				}

				InterlockedIncrement((LONG*)&use_count);
				return &copy_top->obejct;
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
			CRASH();
		if (MEM_GUARD != node->over)
			CRASH();
		if (MEM_GUARD != node->under)
			CRASH();

		if (flag_placementNew) {
			p_obejct->~T();
		}

		for (;;) {
			DWORD64 copy_ABA = top_ABA;

			// top�� �̾���
			node->next_node = (Node*)(copy_ABA & mask);

			// aba count ���� �� new_ABA ����
			DWORD64 aba_count = (copy_ABA + UNUSED_COUNT) & mask_reverse;
			Node* new_ABA = (Node*)(aba_count | (DWORD64)node);

			// ���ÿ� ��ȭ�� �־��ٸ� �ٽýõ�
			if ((DWORD64)copy_ABA != InterlockedCompareExchange64((LONG64*)&top_ABA, (LONG64)new_ABA, (LONG64)copy_ABA))
				continue;

			InterlockedDecrement((LONG*)&use_count);
			return;
		}
	}
}

//------------------------------
// ABA_BitCheck
//------------------------------

//void ABA_BitCheck() {
//	SYSTEM_INFO sys_info;
//	GetSystemInfo(&sys_info);
//
//	// ������ƮǮ ���ο��� ABA �̽��� ���� ���� ������������ ����� �� ���� �޸� �ּ� ���� 17bit�� Ȱ����
//	// ���� ������ �ý����� ������Ʈ�� ���� �������� ���� 17bit�� Ȱ���ϰ� �ȴٸ� nullptr Access�� ���� ũ���� ����.
//	if (((DWORD64)sys_info.lpMaximumApplicationAddress >> (64 - UNUSED_BIT))) {
//		CRASH();
//	}
//}