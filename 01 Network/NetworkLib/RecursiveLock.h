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

// s->s ����� x
// s->e ����� 0, but �ٸ� ������鵵 �� �� ���� �ȵ�. ���� ����
// e->e ����� 0, ��Ͷ� ����ؾ���
// e->s ����� 0, ���� ����, �ָ���

// �Ʒ� �ڵ� ����
//class RecursiveSRWLock {
//public:
//    RecursiveSRWLock() {
//        InitializeSRWLock(&srwlock_);
//        owner_thread_id_ = 0;
//        recursion_count_ = 0;
//    }
//
//    void lock_shared() {
//        DWORD thread_id = GetCurrentThreadId();
//        if (owner_thread_id_ == thread_id) {
//            ++recursion_count_;
//            return;
//        }
//
//        AcquireSRWLockShared(&srwlock_);
//        owner_thread_id_ = thread_id;
//        recursion_count_ = 1;
//    }
//
//    void unlock_shared() {
//        if (--recursion_count_ == 0) {
//            owner_thread_id_ = 0;
//            ReleaseSRWLockShared(&srwlock_);
//        }
//    }
//
//    void lock() {
//        DWORD thread_id = GetCurrentThreadId();
//        if (owner_thread_id_ == thread_id) {
//            ++recursion_count_;
//            return;
//        }
//
//        AcquireSRWLockExclusive(&srwlock_);
//        owner_thread_id_ = thread_id;
//        recursion_count_ = 1;
//    }
//
//    void unlock() {
//        if (--recursion_count_ == 0) {
//            owner_thread_id_ = 0;
//            ReleaseSRWLockExclusive(&srwlock_);
//        }
//    }
//
//private:
//    SRWLOCK srwlock_;
//    DWORD owner_thread_id_;
//    int recursion_count_;
//};