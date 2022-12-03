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
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "../../J_LIB/ObjectPool/ObjectPool.h"
#pragma comment(lib, "../../J_LIB/ObjectPool/ObjectPool.lib")

#include "../../J_LIB/ProtocolBuffer/ProtocolBuffer.h"
#pragma comment(lib, "../../J_LIB/ProtocolBuffer/ProtocolBuffer.lib")

//#include "..\RingBuffer\RingBuffer.h"
//#pragma comment(lib, "..\RingBuffer\RingBuffer.lib")

#define CRASH() do{								\
					char* null_ptr = nullptr;	\
					++(*null_ptr);				\
				}while(false)