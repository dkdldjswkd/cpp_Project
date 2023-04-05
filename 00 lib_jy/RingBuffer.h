#pragma once
#include <memory>
#include <BaseTsd.h>

#define MASKING_8BIT(n)		(0x00ff	& (n)) // 256
#define MASKING_9BIT(n)		(0x01ff & (n)) // 512
#define MASKING_10BIT(n)	(0x03ff & (n)) // 1024
#define MASKING_11BIT(n)	(0x07ff & (n)) // 2048
#define MASKING_12BIT(n)	(0x0fff & (n)) // 4096
#define MASKING_13BIT(n)	(0x1fff & (n)) // 8192

// 버퍼 크기 세팅
#define MASKING_BIT(n)		MASKING_13BIT(n)
#define BUF_SIZE			8192

class RingBuffer {
public:
	RingBuffer();
	~RingBuffer();

private:
	char* begin;
	char* end;

	char* front;
	char* rear;

	char* readPos;
	char* writePos;

private:
	inline int MustEnqueue(const void* src, size_t size);
	inline int MustDequeue(void* dst, size_t size);

	inline void SetReadPos() { readPos = begin + MASKING_BIT((ULONG_PTR)front + 1); }
	inline void SetWritePos() { writePos = begin + MASKING_BIT((ULONG_PTR)rear + 1); }

public:
	inline void MoveFront(int size) { front = begin + MASKING_BIT((ULONG_PTR)front + size); SetReadPos();  }
	inline void MoveRear(int size) { rear = begin + MASKING_BIT((ULONG_PTR)rear + size);    SetWritePos(); }

	inline bool Empty() const { return front == rear; }
	inline bool Full() const { return front == writePos; }
	inline void Clear();

	inline int DirectEnqueueSize() const;
	inline int DirectDequeueSize() const;
	inline int RemainEnqueueSize() const;
	inline int RemainDequeueSize() const;

	// inline 적용 안됨
	inline int GetFreeSize() const;
	inline int GetUseSize() const;

	inline char* GetReadPos() const { return readPos; }
	inline char* GetWritePos()  const { return writePos; }
	inline char* GetBeginPos() const { return begin; }

	int Enqueue(const void* src, size_t size);
	int Dequeue(void* dst, size_t size);
	int Peek(void* dst, size_t size) const;
};

inline int RingBuffer::MustEnqueue(const void* src, size_t size) {
	memmove(writePos, src, size);
	MoveRear(size);
	return size;
}

inline int RingBuffer::MustDequeue(void* dst, size_t size) {
	memmove(dst, readPos, size);
	MoveFront(size);
	return size;
}

inline void RingBuffer::Clear(){
	front = begin;
	rear = begin;

	writePos = front + 1;
	readPos = rear + 1;
}

///////////////////////////////////////////////////////////////////////
// Enqueue, Dequeue Size 는 첫번째 벽까지 계산해야함 
// 
// ex. 첫번째 벽이 시작점(wp, rp) 보다 앞에 있다면, end 까지 계산 후
// 다시 begin 부터 첫번째 벽(wp, rp)까지 계산해야함
///////////////////////////////////////////////////////////////////////

inline int RingBuffer::DirectEnqueueSize() const {
	if (front < writePos) return end - writePos; // 첫번째 벽 (front) 이후
	else return front - writePos;
}

inline int RingBuffer::RemainEnqueueSize() const {
	if (writePos <= front) return 0; // 첫번째 벽 이전
	else return front - begin;
}

inline int RingBuffer::DirectDequeueSize() const {
	if (writePos < readPos) return end - readPos; // 첫번째 벽  (write pos) 이후
	else return writePos - readPos;
}

inline int RingBuffer::RemainDequeueSize() const {
	if (readPos <= writePos) return 0; // 첫번째 벽 이전
	else return writePos - begin;
}

int RingBuffer::GetFreeSize() const {
	return  DirectEnqueueSize() + RemainEnqueueSize();
}
inline int RingBuffer::GetUseSize() const {
	return  DirectDequeueSize() + RemainDequeueSize();
};

inline static int MustPeek(void* dst, const void* src, size_t size) {
	memmove(dst, src, size);
	return size;
}