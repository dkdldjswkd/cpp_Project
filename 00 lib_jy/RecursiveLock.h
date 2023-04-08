#pragma once
#include <Windows.h>

struct RecursiveLock {
public:
	RecursiveLock();
	~RecursiveLock();

public:
	SRWLOCK lock;
	DWORD thread_id = -1;
	int lockCount = 0;

public:
	void Lock();
	void Unlock();
	inline void SharedLock();
	inline void ReleaseSharedLock();
};

inline void RecursiveLock::SharedLock() {
	AcquireSRWLockShared(&lock);
}

inline void RecursiveLock::ReleaseSharedLock() {
	ReleaseSRWLockShared(&lock);
}