#pragma once

#include <Windows.h>
#include <stdarg.h>
#include <memory>
#include <BaseTsd.h>

// * �ش� ObjectPool ���� ȯ�� : Release, x64, ����ȭ ������ OFF
//	���� 12:07 2022-12-12

#define CRASH() do{				\
					int* p = 0;	\
					*p = 0;		\
				}while(false)

constexpr BYTE UNUSED_BIT = 17;

#define MEM_GUARD 0xFDFDFDFDFDFDFDFD

namespace J_LIB {
	template <typename T>
	struct Node;

	template <typename T>
	class ObjectPool {
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
		ObjectPool(int node_num = 0, bool flag_placementNew = false);

		// ���� ���� node delete (������ �������� �ʴ� node�� delete �Ұ�)
		~ObjectPool();

	private:
		const ULONG_PTR integrity = (ULONG_PTR)this;
		bool flag_placementNew = false;
		__declspec(align(64)) DWORD64 unique = 0;
		__declspec(align(64)) DWORD64 unique_top = NULL;
		__declspec(align(64)) int capacity = 0;
		__declspec(align(64)) int use_count = 0;

	public:
		T* Alloc();
		void Free(T* p_obejct);
		inline int GetCapacityCount() const { return capacity; }
		inline int GetUseCount() const { return use_count; }
	};

	//------------------------------
	// ObjectPool
	//------------------------------

	// ������ (* ȣ�� �� ��Ƽ �����忡���� Alloc, Free ȣ���� �־ �ȵ�)
	template<typename T>
	ObjectPool<T>::ObjectPool(int node_num, bool flag_placementNew) :
		integrity((ULONG_PTR)this),
		flag_placementNew(flag_placementNew),

		unique_top(NULL),
		capacity(node_num),
		use_count(0)
	{
		for (int i = 0; i < node_num; i++) {
			Node* new_node = new Node((ULONG_PTR)this);
			new_node->next_node = (Node*)unique_top;
			unique_top = (DWORD64)new_node; 
		}
	}

	// �Ҹ��� (* ȣ�� �� ��Ƽ �����忡���� Alloc, Free ȣ���� �־ �ȵ�)
	template<typename T>
	ObjectPool<T>::~ObjectPool() {
		// ���� 17bit(unique ��) ����
		DWORD64 copy_unique_top = unique_top;
		Node* top = (Node*)((copy_unique_top << UNUSED_BIT) >> UNUSED_BIT);

		for (; top != nullptr;) {
			Node* delete_node = top;
			top = top->next_node;
			delete delete_node;
		}
	}

	// ������ ������ POP�� �ش�
	template<typename T>
	T* ObjectPool<T>::Alloc() {
		DWORD64 unique_num = InterlockedIncrement64((LONG64*)&unique);

		for (;;) {
			DWORD64 copy_unique_top = unique_top;
			Node* copy_top = (Node*)((copy_unique_top << UNUSED_BIT) >> UNUSED_BIT);

			// Not empty!!
			if (copy_top) {
				Node* unique_next = (Node*)((unique_num << (64 - UNUSED_BIT)) | (DWORD64)copy_top->next_node);

				// ���ÿ� ��ȭ�� �־��ٸ� �ٽýõ�
				if (copy_unique_top != InterlockedCompareExchange64((LONG64*)&unique_top, (LONG64)unique_next, (LONG64)copy_unique_top))
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

	// ������ ������ PUSH�� �ش� (ABA ������ �������� ���ÿ� ������ unique ���� ������������)
	template<typename T>
	void ObjectPool<T>::Free(T* p_obejct) {
		// ������Ʈ ���� ��ȯ
		Node* node = (Node*)((char*)p_obejct - sizeof(size_t) - sizeof(ULONG_PTR));

		if (integrity != node->integrity)
			CRASH();
		if (MEM_GUARD != node->over)
			CRASH();
		if (MEM_GUARD != node->under)
			CRASH();

		if (flag_placementNew) {
			p_obejct->~T();
		}

		// Node�� �ּҸ� Unique�ϰ� �ٲ㼭 ���ÿ� ���� (ABA �̽� �ذ�å)
		DWORD64 unique_num = InterlockedIncrement64((LONG64*)&unique);
		Node* unique_node = (Node*)((unique_num << (64 - UNUSED_BIT)) | (DWORD64)node);

		for (;;) {
			DWORD64 copy_unique_top = unique_top;
			Node* copy_top = (Node*)((copy_unique_top << UNUSED_BIT) >> UNUSED_BIT);
			node->next_node = copy_top;

			// ���ÿ� ��ȭ�� �־��ٸ� �ٽýõ�
			if ((DWORD64)copy_unique_top != InterlockedCompareExchange64((LONG64*)&unique_top, (LONG64)unique_node, (LONG64)copy_unique_top))
				continue;

			InterlockedDecrement((LONG*)&use_count);
			return;
		}
	}
}

//------------------------------
// ABA_BitCheck
//------------------------------

void ABA_BitCheck() {
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);

	// ������ƮǮ ���ο��� ABA �̽��� ���� ���� ������������ ����� �� ���� �޸� �ּ� ���� 17bit�� Ȱ����
	// ���� ������ �ý����� ������Ʈ�� ���� �������� ���� 17bit�� Ȱ���ϰ� �ȴٸ� nullptr Access�� ���� ũ���� ����.
	if (((DWORD64)sys_info.lpMaximumApplicationAddress >> (64 - UNUSED_BIT))) {
		CRASH();
	}
}