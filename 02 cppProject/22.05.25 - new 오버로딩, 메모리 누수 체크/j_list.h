#pragma once
#include <iostream>
#include <string>
using namespace std;

struct AllocInfo
{
	void* ptr;
	int size;
	char filename[128];
	int  line;
	bool array;
};

class NODE {
public:
	AllocInfo allocInfo;
	NODE* next;
	NODE* prev;

	NODE() : allocInfo{}, next(0), prev(0) {}
	NODE(void* ptr, int size, const char* filename, int  line, bool array);
	NODE(AllocInfo tmp);
	~NODE() {}
};

class Alloc_List {
	NODE head, tail;

public:
	Alloc_List() {
		head.next = &tail;
		tail.prev = &head;
	}

	~Alloc_List() {
		// 파일 출력
		if (empty()) return;

		FILE* fp_log;
		auto ret = fopen_s(&fp_log, "log.txt", "wt+");
		if (fp_log == NULL) { printf("fp_log == NULL"); return; }
		char buf[256] = {0,};

		NODE* cur = head.next;
		while (cur != &tail) {
			sprintf_s(buf, 256, "LEAK [0x%p] [%d] %s : %d \n", cur->allocInfo.ptr, cur->allocInfo.size, cur->allocInfo.filename, cur->allocInfo.line);
			fwrite(buf, strlen(buf), 1, fp_log);
			cur = cur->next;
		};

		fclose(fp_log);
	}

	void Init();
	bool empty();
	bool Add(void* ptr, int size, const char* filename, int  line, bool array);
	bool Remove(void* ptr);
};

class Memory_Manager {
public:
	Memory_Manager() {}
	~Memory_Manager() {}

public:
	Alloc_List list;

public:
	bool Allocation(void* ptr, int size, const char* filename, int  line, bool array);
	bool free(void* ptr);
};

extern Memory_Manager mem_manager;