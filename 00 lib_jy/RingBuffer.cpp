#include "RingBuffer.h"
#include <stdlib.h>
#include <BaseTsd.h>

RingBuffer::RingBuffer(){
	begin = (char*)_aligned_malloc(BUF_SIZE, BUF_SIZE);
	end = begin + BUF_SIZE;

	front = begin;
	rear = begin;

	writePos = front + 1;
	readPos = rear + 1;
}

RingBuffer::~RingBuffer() {
	_aligned_free(begin);
}

///////////////////////////////////////////////////////////////////////
// Enqueue, Dequeue 공통사항
// 1. write_pos, read_pos이 첫번째 벽 이전 위치일때
//		* 1-1. 읽을 수 있는 최대크기는 Direct Size 이다.
//	  *** 1-2. 즉, Remain Size는 0이다
// 
// 2. write_pos, read_pos이 첫번째 벽 이후 위치일때
//		* 2-1. Direct Size는 pos 부터 두번재 벽 까지이다.
//		* 2-2. Reamin Size는 0 이상 이다.
// 
// *** 첫번째 벽은 write pos 기준	: 'front'
//				   read pos  기준	: 'write pos'
//	   두번째 벽은 read/write 동일	: 'end'
///////////////////////////////////////////////////////////////////////

int RingBuffer::Enqueue(const void* src, size_t size) {
	int free_size = GetFreeSize();
	if (free_size < size)
		size = free_size;

	const int direct_size = DirectEnqueueSize();

	if (size <= direct_size) {
		return MustEnqueue(src, size);
	}
	else {
		MustEnqueue(src, direct_size);
		return direct_size + MustEnqueue((char*)src + direct_size, size - direct_size);
	}
}

int RingBuffer::Dequeue(void* dst, size_t size) {
	int use_size = GetUseSize();
	if (use_size < size)
		size = use_size;

	const int direct_size = DirectDequeueSize();

	if (size <= direct_size) {
		return MustDequeue(dst, size);
	}
	else {
		MustDequeue(dst, direct_size);
		return direct_size + MustDequeue((char*)dst + direct_size, size - direct_size);
	}
}

int RingBuffer::Peek(void* dst, size_t size) const {
	int use_size = GetUseSize();
	if (use_size < size)
		size = use_size;

	const int direct_size = DirectDequeueSize();

	if (size <= direct_size) {
		return MustPeek(dst, readPos, size);
	}
	else {
		MustPeek(dst, readPos, direct_size);
		return direct_size + MustPeek((char*)dst + direct_size, begin, size - direct_size);
	}
}