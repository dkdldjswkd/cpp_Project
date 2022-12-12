#include <Windows.h>
#include "MemoryNote.h"

#define NOTE_BYTE 10000000

MemoryNote::MemoryNote() {
	begin = (char*)malloc(NOTE_BYTE * sizeof(char));
	wp = begin - 8; // 8byte �� write �ϱ� ���� - 8
	end = begin + NOTE_BYTE;
}

char* MemoryNote::get_wp() {
	return (char*)InterlockedAdd64((long long*)&wp, 8);
}
