#pragma once

struct MemoryNote {
public:
	MemoryNote(int log_size, int note_size = 134217738);
public:
	char* begin;
	char* end;
	int log_size;
	int note_size;

	DWORD offset = 0;
	DWORD mask;
public:
	char* get_wp();
};