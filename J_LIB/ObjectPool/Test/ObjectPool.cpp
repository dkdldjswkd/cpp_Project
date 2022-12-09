#include "ObjectPool.h"

//------------------------------
// ABA_BitCheck
//------------------------------

// 유저영역의 상위 17bit를 유저영역에서 여전히 사용하지 않는지
// 런타임에 체크

ABA_BitCheck::ABA_BitCheck() {
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);

	// 오브젝트풀 내부에서 ABA 이슈를 막기 위해 유저영역에서 사용할 수 없는 메모리 주소 상위 17bit를 활용함
	// 만약 윈도우 시스템의 업데이트로 인해 유저영역 상위 17bit를 활용하게 된다면 nullptr Access로 인한 크래시 유도.
	if (((DWORD64)sys_info.lpMaximumApplicationAddress >> (64 - UNUSED_BIT))) {
		CRASH();
	}
}

ABA_BitCheck ABA_BitCheck::inst;