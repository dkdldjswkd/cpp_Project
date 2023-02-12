#include "RecursiveLock.h"

RecursiveLock::RecursiveLock() {
	InitializeSRWLock(&lock);
}

RecursiveLock::~RecursiveLock() {
}