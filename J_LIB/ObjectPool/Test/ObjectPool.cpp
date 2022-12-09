#include "ObjectPool.h"

//------------------------------
// ABA_BitCheck
//------------------------------

// ���������� ���� 17bit�� ������������ ������ ������� �ʴ���
// ��Ÿ�ӿ� üũ

ABA_BitCheck::ABA_BitCheck() {
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);

	// ������ƮǮ ���ο��� ABA �̽��� ���� ���� ������������ ����� �� ���� �޸� �ּ� ���� 17bit�� Ȱ����
	// ���� ������ �ý����� ������Ʈ�� ���� �������� ���� 17bit�� Ȱ���ϰ� �ȴٸ� nullptr Access�� ���� ũ���� ����.
	if (((DWORD64)sys_info.lpMaximumApplicationAddress >> (64 - UNUSED_BIT))) {
		CRASH();
	}
}

ABA_BitCheck ABA_BitCheck::inst;