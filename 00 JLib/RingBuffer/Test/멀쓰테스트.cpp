#include "../RingBuffer.h"
#include <iostream>
#include <Windows.h>
#include <thread>
#pragma comment(lib, "../RingBuffer.lib")

using namespace std;

const char* str = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 123451234567890 abcdefghijklmnopqrstuvwxyz 1";
int str_size = strlen(str); //120
RingBuffer g_RingBuffer;

void EnqueueTest() {
	for (;;) {
		if (g_RingBuffer.Get_FreeSize() >= str_size)
			g_RingBuffer.Enqueue(str, str_size);
		Sleep(200);
	}
}

void DequeueTest(){
    for (;;)    {
        int randSize = rand() % str_size;
        char buffer[128];
        int ret = g_RingBuffer.Dequeue(buffer, randSize);
        buffer[ret] = '\0';
        std::cout << buffer;
        Sleep(200);
    }
}

int main(){
    thread enqueue_thread(EnqueueTest);
    thread dequeue_thread(DequeueTest);

    enqueue_thread.join();   // �����尡 ���ѷ��� ���Ƽ� ��ǻ� ȣ����� ����
    dequeue_thread.join();   // �����尡 ���ѷ��� ���Ƽ� ��ǻ� ȣ����� ����
    return 0;
}