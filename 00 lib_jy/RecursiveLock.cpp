#include "RecursiveLock.h"

RecursiveLock::RecursiveLock() {
	InitializeSRWLock(&lock);
}

RecursiveLock::~RecursiveLock() {
}

// Àç±Í lock ±¸Çö
void RecursiveLock::Lock() {
	auto id = GetCurrentThreadId();

	// Àç±Í lock ÆÇ´Ü
	if (id == thread_id) {
		lockCount++;
		return;
	}
	else {
		AcquireSRWLockExclusive(&lock);
		lockCount++;
		thread_id = id;
	}
}

// Àç±Í unlock ±¸Çö
void RecursiveLock::Unlock() {
	// Àç±Í unlock ÆÇ´Ü
	if (1 < lockCount) {
		lockCount--;
		return;
	}
	thread_id = -1;
	lockCount--;
	ReleaseSRWLockExclusive(&lock);
}