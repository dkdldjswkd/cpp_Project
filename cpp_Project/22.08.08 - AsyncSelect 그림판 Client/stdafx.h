#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <Windows.h>
#include <string>
#include "RingBuffer.h"

#define CRASH() do{								\
					char* null_ptr = nullptr;	\
					++(*null_ptr);				\
				}while(false)

