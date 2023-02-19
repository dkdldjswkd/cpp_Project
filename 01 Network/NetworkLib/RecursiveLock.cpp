#include "RecursiveLock.h"

RecursiveLock::RecursiveLock() {
	InitializeSRWLock(&lock);
}

RecursiveLock::~RecursiveLock() {
}

// ��� lock ����
void RecursiveLock::Lock_Exclusive() {
	AcquireSRWLockExclusive(&lock);

	// ��� �� ����
	//auto id = GetCurrentThreadId();

	//// ��� lock �Ǵ�
	//if (id == thread_id) {
	//	lockCount++;
	//	return;
	//}
	//else {
	//	AcquireSRWLockExclusive(&lock);
	//	lockCount++;
	//	thread_id = id;
	//}
}

// ��� unlock ����
void RecursiveLock::Unlock_Exclusive() {
	ReleaseSRWLockExclusive(&lock);

	// ��� �� ����
	//// ��� unlock �Ǵ�
	//if (1 < lockCount) {
	//	lockCount--;
	//	return;
	//}
	//thread_id = -1;
	//lockCount--;
	//ReleaseSRWLockExclusive(&lock);
}