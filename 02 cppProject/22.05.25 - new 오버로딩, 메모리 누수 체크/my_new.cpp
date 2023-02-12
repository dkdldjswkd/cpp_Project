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

// ���� �߻��� ȣ��Ǵ� delete, ������� ����
void operator delete (void* p, char* File, int Line) {
	// ����� �ڵ�
	cout << "void operator delete (void* p, char* File, int Line)" << endl;
}

void operator delete[](void* p, char* File, int Line) {
	// ����� �ڵ�
	cout << "void operator delete[](void* p, char* File, int Line)" << endl;
}
