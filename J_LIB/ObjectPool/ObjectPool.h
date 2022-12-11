#pragma once

#include <Windows.h>
#include <stdarg.h>
#include <memory>
#include <BaseTsd.h>

// * 해당 ObjectPool 구현 환경 : Release, x64, 최적화 컴파일 OFF
//	오전 12:07 2022-12-12

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
			// [코드][언더] [ OBJECT ] [오버] [ NEXT* ]
		public:
			const ULONG_PTR integrity;
			const size_t under = MEM_GUARD;
			T obejct;
			const size_t over = MEM_GUARD;
			Node* next_node = nullptr;
		};

	public:
		// node_num 만큼 node 동적할당, flag set (true : Alloc 시 마다 생성자 call, false : 노드 생성 시 최초 1회 생성자 call)
		ObjectPool(int node_num = 0, bool flag_placementNew = false);

		// 스택 보유 node delete (스택이 보유하지 않는 node는 delete 불가)
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

	// 생성자 (* 호출 시 멀티 스레드에서의 Alloc, Free 호출이 있어선 안됨)
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

	// 소멸자 (* 호출 시 멀티 스레드에서의 Alloc, Free 호출이 있어선 안됨)
	template<typename T>
	ObjectPool<T>::~ObjectPool() {
		// 상위 17bit(unique 값) 날림
		DWORD64 copy_unique_top = unique_top;
		Node* top = (Node*)((copy_unique_top << UNUSED_BIT) >> UNUSED_BIT);

		for (; top != nullptr;) {
			Node* delete_node = top;
			top = top->next_node;
			delete delete_node;
		}
	}

	// 락프리 스택의 POP에 해당
	template<typename T>
	T* ObjectPool<T>::Alloc() {
		DWORD64 unique_num = InterlockedIncrement64((LONG64*)&unique);

		for (;;) {
			DWORD64 copy_unique_top = unique_top;
			Node* copy_top = (Node*)((copy_unique_top << UNUSED_BIT) >> UNUSED_BIT);

			// Not empty!!
			if (copy_top) {
				Node* unique_next = (Node*)((unique_num << (64 - UNUSED_BIT)) | (DWORD64)copy_top->next_node);

				// 스택에 변화가 있었다면 다시시도
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

				// Node 중 Object 포인터 ret
				return &(new Node((ULONG_PTR)this))->obejct;
			}
		}
	}

	// 락프리 스택의 PUSH에 해당 (ABA 문제를 막기위해 스택에 꽂을때 unique 값을 증가시켜주자)
	template<typename T>
	void ObjectPool<T>::Free(T* p_obejct) {
		// 오브젝트 노드로 변환
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

		// Node의 주소를 Unique하게 바꿔서 스택에 꼽음 (ABA 이슈 해결책)
		DWORD64 unique_num = InterlockedIncrement64((LONG64*)&unique);
		Node* unique_node = (Node*)((unique_num << (64 - UNUSED_BIT)) | (DWORD64)node);

		for (;;) {
			DWORD64 copy_unique_top = unique_top;
			Node* copy_top = (Node*)((copy_unique_top << UNUSED_BIT) >> UNUSED_BIT);
			node->next_node = copy_top;

			// 스택에 변화가 있었다면 다시시도
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

	// 오브젝트풀 내부에서 ABA 이슈를 막기 위해 유저영역에서 사용할 수 없는 메모리 주소 상위 17bit를 활용함
	// 만약 윈도우 시스템의 업데이트로 인해 유저영역 상위 17bit를 활용하게 된다면 nullptr Access로 인한 크래시 유도.
	if (((DWORD64)sys_info.lpMaximumApplicationAddress >> (64 - UNUSED_BIT))) {
		CRASH();
	}
}