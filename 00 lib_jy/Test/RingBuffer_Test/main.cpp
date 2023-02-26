#include <iostream>
#include <Windows.h>

#include "../RingBuffer.h"
#pragma comment(lib, "../RingBuffer.lib")

#include <timeapi.h>
#pragma comment(lib, "Winmm.lib")

using namespace std;

int main()
{
    RingBuffer ringBuffer;
    const char* data = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 123451234567890 abcdefghijklmnopqrstuvwxyz 1";
    int size = strlen(data);
    ringBuffer.Enqueue(data, size);

    char peek_buf[128];
    char dequeue_buf[128];

    int cpy_size = 0;

    auto start = timeGetTime();

    for (int i = 1; i < 10000000; i++)
    {
        int old_randSize = cpy_size;

        cpy_size = (rand() % (size - 1)) + 1; // 1~119

        auto ret_peek = ringBuffer.Peek(peek_buf, cpy_size);
        if (cpy_size != ret_peek)
        {
             std::cout << "Peek error" << std::endl;
        }
        auto ret_Dequeue = ringBuffer.Dequeue(dequeue_buf, cpy_size);
        if (cpy_size != ret_Dequeue)
        {
              std::cout << "Dequeue error" << std::endl;
            break;
        }
        if (memcmp(peek_buf, dequeue_buf, cpy_size) != 0)
        {
               std::cout << "Not same" << std::endl;
            break;
        }
        auto ret_enqueue = ringBuffer.Enqueue(peek_buf, cpy_size);
        if (cpy_size != ret_enqueue)
        {
              std::cout << "Enqueue error" << std::endl;
            break;
        }
        peek_buf[cpy_size] = '\0';
        std::cout << peek_buf;
        Sleep(100);
    }

    auto end = timeGetTime();
    cout << end - start << endl;

    return 0;
}