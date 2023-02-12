#pragma once
#include <Windows.h>

struct RecursiveLock {
public:
	RecursiveLock();
	~RecursiveLock();

public:
	SRWLOCK lock;
	alignas(64) bool preempted = false;
	alignas(64) DWORD thread_id = -1;
	alignas(64) int lockCount = 0;

public:
	void Lock_Shared() {
		if (InterlockedExchange8((CHAR*)&preempted, (CHAR)true)) {
			// Àç±Í Lock
			if (thread_id == GetCurrentThreadId()) {
				InterlockedIncrement((LONG*)&lockCount);
				return;
			}
		}

		AcquireSRWLockShared(&lock);
		thread_id = GetCurrentThreadId();
	}

	void Unlock_Shared() {
		if (0 == InterlockedDecrement((LONG*)&lockCount)) {
			// Unlock
			ReleaseSRWLockShared(&lock);
			InterlockedExchange8((CHAR*)&preempted, (CHAR)false);
		}
	}

	void Lock_Exclusive() {
		AcquireSRWLockExclusive(&lock);
	}

	void Unock_Exclusive() {
		ReleaseSRWLockExclusive(&lock);
	}
};


