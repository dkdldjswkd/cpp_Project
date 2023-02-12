#include "stdafx.h"
#include "RingBuffer.h"
#include <stdlib.h>

RingBuffer::RingBuffer(int size) :buf_size(size), free_size(size), use_size(0) {
	front = (char*)malloc(size + 1);
	rear = front;

	begin = front;
	end = begin + size;
}

int RingBuffer::Must_Dequeue(char* dst, int size) {
	memmove(dst, front + 1, size);

	front += size;
	free_size += size;
	use_size -= size;

	return size;
}

int RingBuffer::Must_Enqueue(const char* src, int size) {
	memmove(rear + 1, src, size);

	rear += size;
	free_size -= size;
	use_size += size;

	return size;
}

// return : 넣은 데이터 크기만큼 반환
int RingBuffer::Enqueue(const char* src, int size) {
	int direct_enqueue_size = DirectEnqueueSize();

	if (size <= direct_enqueue_size) {
		return Must_Enqueue(src, size);
	}
	else {
		Must_Enqueue(src, direct_enqueue_size);
		if (!Full()) {
			rear = begin - 1;
			return direct_enqueue_size + Enqueue(src + direct_enqueue_size, size - direct_enqueue_size);
		}
		else {
			return direct_enqueue_size;
		}
	}
}

int RingBuffer::Dequeue(char* dst, int size) {
	int direct_dequeue_size = DirectDequeueSize();

	if (size <= direct_dequeue_size) {
		return Must_Dequeue(dst, size);
	}
	else {
		Must_Dequeue(dst, direct_dequeue_size);
		if (!Empty()) {
			front = begin - 1;
			return direct_dequeue_size + Dequeue(dst + direct_dequeue_size, size - direct_dequeue_size);
		}
		else {
			return direct_dequeue_size;
		}
	}
}

int RingBuffer::Peek(char* dst, int size) const {
	int direct_dequeue_size = DirectDequeueSize();
	if (size <= direct_dequeue_size) {
		memmove(dst, front + 1, size);
		return size;
	}

	// 복사해 줄 데이터가 없다면 ret
	memmove(dst, front + 1, direct_dequeue_size);
	if (use_size - direct_dequeue_size <= 0) {
		return direct_dequeue_size;
	}

	// 복사해줄 데이터가 더 있다면 **버퍼가 뒤집혔다는 뜻
	int remain_data_size = size - direct_dequeue_size;
	int use_data_size = use_size - direct_dequeue_size;

	// 남은 데이터보다 복사해줄 데이터가 더 많다면, 전부 복사해줌
	if (use_data_size < remain_data_size) {
		return direct_dequeue_size + Peek_B_Second(dst + direct_dequeue_size);
	}
	// 복사해줄 데이터가 있는경우
	else {
		memmove(dst + direct_dequeue_size, begin, remain_data_size);
		return direct_dequeue_size + remain_data_size;
	}
}

void RingBuffer::ClearBuffer() {
	front = begin;
	rear = begin;

	free_size = buf_size;
	use_size = 0;
}

bool RingBuffer::MoveRear(int size) {
	// 정방향
	if (front <= rear) {
		int direct_move_size = end - rear;
		// 정방향 유지
		if (size <= direct_move_size) {
			rear += size;
			free_size -= size;
			use_size += size;
			return true;
		}
		// 역방향으로 뒤집힘
		else {
			char* estimated_rear_location = begin + (size - direct_move_size - 1);
			if (front < estimated_rear_location) {
				return false;
			}
			rear = estimated_rear_location;
			free_size -= size;
			use_size += size;
			return false;
		}
	}
	// 역방향 (역방향 유지)
	else {
		int can_move_size = front - rear;
		if (can_move_size < size) {
			return false;
		}
		else {
			rear += size;
			free_size -= size;
			use_size += size;
			return true;
		}
	}
}

bool RingBuffer::MoveFront(int size) {
	// 정방향 (정방향 유지)
	if (front <= rear) {
		int can_move_size = rear - front;
		if (can_move_size < size) {
			return false;
		}
		else {
			front += size;
			free_size += size;
			use_size -= size;
			return true;
		}
	}
	// 역방향
	else {
		int direct_move_size = end - front;
		// 역방향 유지
		if (size <= direct_move_size) {
			front += size;
			free_size += size;
			use_size -= size;
			return true;
		}
		// 정방향으로 뒤집힘
		else {
			char* estimated_front_location = begin + (size - direct_move_size - 1);
			if (rear < estimated_front_location) {
				return false;
			}
			front = estimated_front_location;
			free_size += size;
			use_size -= size;
			return true;
		}
	}
}