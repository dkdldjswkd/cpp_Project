#include "my_new.h"

#undef new

void* operator new (size_t size, const char* FILE, int Line) {
	//cout << size << endl;

	void* ptr = malloc(size);
	mem_manager.Allocation(ptr, size, FILE, Line, false);
	return ptr;
}

void operator delete (void* p) {
	mem_manager.free(p);
	free(p);
}

void* operator new[](size_t size, const char* FILE, int Line) {
	cout << size << endl;
	void* ptr = malloc(size);
	mem_manager.Allocation(ptr, size, FILE, Line, true);
	return ptr;
}

void operator delete[](void* p) {
	mem_manager.free(p);
	free(p);
}

// 예외 발생시 호출되는 delete, 사용하지 않음
void operator delete (void* p, char* File, int Line) {
	// 디버그 코드
	cout << "void operator delete (void* p, char* File, int Line)" << endl;
}

void operator delete[](void* p, char* File, int Line) {
	// 디버그 코드
	cout << "void operator delete[](void* p, char* File, int Line)" << endl;
}
