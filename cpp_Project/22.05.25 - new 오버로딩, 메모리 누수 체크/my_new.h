#pragma once
#include "j_list.h"

// 디버그용
#include <iostream>
using namespace std;

void* operator new (size_t size, const char* FILE, int Line);
void* operator new[](size_t size, const char* FILE, int Line);
// 짝 맞추기 위함
void operator delete (void* p, char* File, int Line);
void operator delete[](void* p, char* File, int Line);

void operator delete (void* p);
void operator delete[](void* p);

#define new new(__FILE__, __LINE__)