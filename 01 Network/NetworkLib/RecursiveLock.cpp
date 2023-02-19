#include "RecursiveLock.h"

RecursiveLock::RecursiveLock() {
	InitializeSRWLock(&lock);
}

RecursiveLock::~RecursiveLock() {
}

// Àç±Í lock ±¸Çö
void RecursiveLock::Lock_Exclusive() {
	AcquireSRWLockExclusive(&lock);

	// Àç±Í ¶ô ¹öÀü
	//auto id = GetCurrentThreadId();

	//// Àç±Í lock ÆÇ´Ü
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

// Àç±Í unlock ±¸Çö
void RecursiveLock::Unlock_Exclusive() {
	ReleaseSRWLockExclusive(&lock);

	// Àç±Í ¶ô ¹öÀü
	//// Àç±Í unlock ÆÇ´Ü
	//if (1 < lockCount) {
	//	lockCount--;
	//	return;
	//}
	//thread_id = -1;
	//lockCount--;
	//ReleaseSRWLockExclusive(&lock);
}