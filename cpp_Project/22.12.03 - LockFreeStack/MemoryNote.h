#pragma once

struct MemoryNote {
public:
	MemoryNote();
public:
	char* begin;
	char* wp;
	char* end;

public:
	char* get_wp();
};