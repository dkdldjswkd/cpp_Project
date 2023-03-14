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
	void Lock_Exclusive();
	void Unlock_Exclusive();
	inline void Lock_Shared();
	inline void Unlock_Shared();
};

inline void RecursiveLock::Lock_Shared() {
	AcquireSRWLockShared(&lock);
}

inline void RecursiveLock::Unlock_Shared() {
	ReleaseSRWLockShared(&lock);
}