#include "j_list.h"

NODE::NODE(void* ptr, int size, const char* filename, int  line, bool array) {
	allocInfo.ptr = ptr;
	allocInfo.size = size;
	strcpy_s(allocInfo.filename, 128, filename);
	allocInfo.line = line;
	allocInfo.array = array;

	next = NULL;
	prev = NULL;
}

NODE::NODE(AllocInfo tmp) {
	allocInfo.ptr = tmp.ptr;
	allocInfo.size = tmp.size;
	strcpy_s(allocInfo.filename, 128, tmp.filename);
	allocInfo.line = tmp.line;
	allocInfo.array = tmp.array;

	next = NULL;
	prev = NULL;
}

void Alloc_List::Init() {
	NODE* ptr;
	while (head.next != &tail) {
		ptr = head.next;
		head.next = head.next->next;
		free(ptr);
	}
}

bool Alloc_List::empty() {
	if (head.next == &tail)
		return true;
	return false;
}

bool Alloc_List::Add(void* ptr, int size, const char* filename, int  line, bool array) {
	if (array == true) size += 8;

	NODE* node = new NODE(ptr, size, filename, line, array);
	node->next = &tail;
	node->prev = tail.prev;

	node->prev->next = node;
	node->next->prev = node;

	return true;
}

bool Alloc_List::Remove(void* ptr)
{
	NODE* cur = head.next;

	while (cur != &tail) {
		if (cur->allocInfo.ptr == ptr) {
			cur->prev->next = cur->next;
			cur->next->prev = cur->prev;
			free(cur);
			return true;
		}
		cur = cur->next;
	}

	return false;
}

bool Memory_Manager::Allocation(void* ptr, int size, const char* filename, int  line, bool array) {
	// 파일 출력
	return list.Add(ptr, size, filename, line, array);
}

bool Memory_Manager::free(void* ptr) {
	return list.Remove(ptr);
}

Memory_Manager mem_manager;