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

// s->s 데드락 x
// s->e 데드락 0, but 다른 스레드들도 들어갈 수 들어가면 안됨. 로직 오류
// e->e 데드락 0, 재귀락 허용해야함
// e->s 데드락 0, 로직 오류, 애매함