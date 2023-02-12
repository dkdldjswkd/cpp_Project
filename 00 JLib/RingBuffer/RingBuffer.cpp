#include "RingBuffer.h"
#include <stdlib.h>
#include <BaseTsd.h>

RingBuffer::RingBuffer(){
	begin = (char*)_aligned_malloc(BUF_SIZE, BUF_SIZE);
	end = begin + BUF_SIZE;

	front = begin;
	rear = begin;

	write_pos = front + 1;
	read_pos = rear + 1;
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
	int free_size = Get_FreeSize();
	if (free_size < size)
		size = free_size;

	const int direct_size = Direct_EnqueueSize();

	if (size <= direct_size) {
		return Must_Enqueue(src, size);
	}
	else {
		Must_Enqueue(src, direct_size);
		return direct_size + Must_Enqueue((char*)src + direct_size, size - direct_size);
	}
}

int RingBuffer::Dequeue(void* dst, size_t size) {
	int use_size = Get_UseSize();
	if (use_size < size)
		size = use_size;

	const int direct_size = Direct_DequeueSize();

	if (size <= direct_size) {
		return Must_Dequeue(dst, size);
	}
	else {
		Must_Dequeue(dst, direct_size);
		return direct_size + Must_Dequeue((char*)dst + direct_size, size - direct_size);
	}
}

int RingBuffer::Peek(void* dst, size_t size) const {
	int use_size = Get_UseSize();
	if (use_size < size)
		size = use_size;

	const int direct_size = Direct_DequeueSize();

	if (size <= direct_size) {
		return Must_Peek(dst, read_pos, size);
	}
	else {
		Must_Peek(dst, read_pos, direct_size);
		return direct_size + Must_Peek((char*)dst + direct_size, begin, size - direct_size);
	}
}