#pragma once

#include <Windows.h>
#include <stdarg.h>
#include <memory>
#include <BaseTsd.h>
// LFObjectPool - 원본

// * 해당 LFObjectPool 구현 환경 : Release, x64, 최적화 컴파일 OFF
//	오전 3:14 2022-12-18

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
		LFObjectPool(int node_num = 0, bool flag_placementNew = false);

		// 스택 보유 node delete (스택이 보유하지 않는 node는 delete 불가)
		~LFObjectPool();

	private:
		const ULONG_PTR integrity = (ULONG_PTR)this;
		bool flag_placementNew = false;
		__declspec(align(64)) DWORD64 top_ABA = NULL;
		__declspec(align(64)) int capacity = 0;
		__declspec(align(64)) int use_count = 0;
		int object_offset = 0;
		DWORD64 mask		 = 0x00007FFFFFFFFFFF; // 상위 17bit 0
		DWORD64 mask_reverse = 0xFFFF800000000000; // 상위 17bit 0

	public:
		T* Alloc();
		void Free(T* p_obejct);
		inline int GetCapacityCount() const { return capacity; }
		inline int Get_UseCount() const { return use_count; }
	};

	//------------------------------
	// ObjectPool
	//------------------------------

	// 생성자 (* 호출 시 멀티 스레드에서의 Alloc, Free 호출이 있어선 안됨)
	template<typename T>
	LFObjectPool<T>::LFObjectPool(int node_num, bool flag_placementNew) :
		integrity((ULONG_PTR)this),
		flag_placementNew(flag_placementNew),

		top_ABA(NULL),
		capacity(node_num),
		use_count(0)
	{
		// object_offset를 셋팅하기 위해 Node 생성
		Node* new_node = new Node((ULONG_PTR)this);
		new_node->next_node = (Node*)top_ABA;
		top_ABA = (DWORD64)new_node;
		object_offset = ((int)(&new_node->obejct) - (int)new_node);
		capacity++;

		// 요청한 노드 개수만큼 미리 노드 생성
		for (int i = 0; i < node_num - 1; i++) {
			Node* new_node = new Node((ULONG_PTR)this);
			new_node->next_node = (Node*)top_ABA;
			top_ABA = (DWORD64)new_node;
		}
	}

	// 소멸자 (* 호출 시 멀티 스레드에서의 Alloc, Free 호출이 있어선 안됨)
	template<typename T>
	LFObjectPool<T>::~LFObjectPool() {
		Node* top = (Node*)(top_ABA & mask);

		for (; top != nullptr;) {
			Node* delete_node = top;
			top = top->next_node;
			delete delete_node;
		}
	}

	// 락프리 스택의 POP에 해당
	template<typename T>
	T* LFObjectPool<T>::Alloc() {
		for (;;) {
			DWORD64 copy_ABA = top_ABA;
			Node* copy_top = (Node*)(copy_ABA & mask);

			// Not empty!!
			if (copy_top) {
				// aba count 추출 및 new_ABA 생성
				DWORD64 aba_count = (copy_ABA + UNUSED_COUNT) & mask_reverse;
				Node* new_ABA = (Node*)(aba_count | (DWORD64)copy_top->next_node);

				// 스택에 변화가 있었다면 다시시도
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

				// Node 중 Object 포인터 ret
				return &(new Node((ULONG_PTR)this))->obejct;
			}
		}
	}

	// 락프리 스택의 PUSH에 해당
	template<typename T>
	void LFObjectPool<T>::Free(T* p_obejct) {
		// 오브젝트 노드로 변환
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

			// top에 이어줌
			node->next_node = (Node*)(copy_ABA & mask);

			// aba count 추출 및 new_ABA 생성
			DWORD64 aba_count = (copy_ABA + UNUSED_COUNT) & mask_reverse;
			Node* new_ABA = (Node*)(aba_count | (DWORD64)node);

			// 스택에 변화가 있었다면 다시시도
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
//	// 오브젝트풀 내부에서 ABA 이슈를 막기 위해 유저영역에서 사용할 수 없는 메모리 주소 상위 17bit를 활용함
//	// 만약 윈도우 시스템의 업데이트로 인해 유저영역 상위 17bit를 활용하게 된다면 nullptr Access로 인한 크래시 유도.
//	if (((DWORD64)sys_info.lpMaximumApplicationAddress >> (64 - UNUSED_BIT))) {
//		CRASH();
//	}
//}