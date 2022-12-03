#include <iostream>
#include <thread>
using namespace std;

#define CRASH()		do{						\
						int* p = nullptr;	\
						int a = *p;			\
					}while(true); 

int main() {
	int a = 1;
	CRASH();
	a = 2;
}