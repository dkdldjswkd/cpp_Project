#include <Windows.h>
#include "MemoryNote.h"

#define CRASH() do{				\
					int* p = 0;	\
					*p = 0;		\
				}while(false)

MemoryNote::MemoryNote(int log_size, int note_size ) : log_size(log_size), note_size(note_size) {
	mask = 0xffffffff >> 5;

	begin = (char*)malloc(note_size * sizeof(char));
	end = begin + note_size;

	offset = -log_size;
}

char* MemoryNote::get_wp() {
	auto tmp = begin + (InterlockedAdd((LONG*)&offset, (LONG64)log_size) & mask);
	return tmp;
}