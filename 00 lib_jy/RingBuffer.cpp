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
// Enqueue, Dequeue �������
// 1. write_pos, read_pos�� ù��° �� ���� ��ġ�϶�
//		* 1-1. ���� �� �ִ� �ִ�ũ��� Direct Size �̴�.
//	  *** 1-2. ��, Remain Size�� 0�̴�
// 
// 2. write_pos, read_pos�� ù��° �� ���� ��ġ�϶�
//		* 2-1. Direct Size�� pos ���� �ι��� �� �����̴�.
//		* 2-2. Reamin Size�� 0 �̻� �̴�.
// 
// *** ù��° ���� write pos ����	: 'front'
//				   read pos  ����	: 'write pos'
//	   �ι�° ���� read/write ����	: 'end'
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