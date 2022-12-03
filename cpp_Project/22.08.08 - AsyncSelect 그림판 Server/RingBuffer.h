#pragma once

#include <memory>

class RingBuffer {
public:
	RingBuffer(int size);

private:
	const unsigned int buf_size;
	unsigned int free_size;
	unsigned int use_size;
	char* front;
	char* rear;
	char* begin;
	char* end;

public:

private:
	// *** 꼭 !!! *** 버퍼 방향이 정방향일때만 (ALL : front 부터 rear 까지 전부)
	inline int Peek_F_ALL(char* dst) const { memmove(dst, front + 1, rear - front); return  rear - front; }
	// *** 꼭 !!! *** 버퍼 방향이 역방향일때만 (First : front 부터,  Second : 잘린부분 부터)
	inline int Peek_B_First(char* dst) const { memmove(dst, front + 1, end - front); return end - front; }
	inline int Peek_B_Second(char* dst) const { memmove(dst, begin, rear - begin + 1); return  rear - begin + 1; }

	int Size_From_Head() const { return rear - begin + 1; };
	int Must_Dequeue(char* dst, int size);
	int Must_Enqueue(const char* src, int size);


public:
	// enqueue 가능 크기 반환 (full : ret 0)
	int DirectEnqueueSize() const { return front <= rear ? end - rear : front - rear - 1; }
	// dequeue 가능 크기 반환 (empty : ret 0)
	int DirectDequeueSize() const { return front <= rear ? rear - front : end - front; }
	// ***버퍼가 비어있을 수 있음
	inline bool Is_Forward() const { return front <= rear; };

	inline bool Empty() const { return front == rear; }
	inline bool Full() const { return rear == (front - 1); }

	inline int GetFreeSize() const { return free_size; }
	inline int GetUseSize() const { return use_size; }
	int Enqueue(const char* src, int size);
	int Dequeue(char* dst, int size);
	int Peek(char* dst, int size) const;
	void ClearBuffer();
	bool MoveRear(int size);
	bool MoveFront(int size);

	inline char* GetFrontBufferPtr() const { return front; }
	inline char* GetRearBufferPtr() const { return rear; }
};
