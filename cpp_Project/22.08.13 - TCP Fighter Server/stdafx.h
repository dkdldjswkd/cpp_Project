#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <Windows.h>
#pragma comment(lib, "winmm.lib")

#include <iostream>
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>

#define CRASH() do{								\
					char* null_ptr = nullptr;	\
					++(*null_ptr);				\
				}while(false)