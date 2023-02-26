#pragma once
#include <memory>
#include <BaseTsd.h>

#define MASKING_8BIT(n)		(0x00ff	& (n)) // 256
#define MASKING_9BIT(n)		(0x01ff & (n)) // 512
#define MASKING_10BIT(n)	(0x03ff & (n)) // 1024
#define MASKING_11BIT(n)	(0x07ff & (n)) // 2048
#define MASKING_12BIT(n)	(0x0fff & (n)) // 4096
#define MASKING_13BIT(n)	(0x1fff & (n)) // 8192

// ���� ũ�� ����
#define MASKING_BIT(n)		MASKING_13BIT(n)
#define BUF_SIZE			8192

class RingBuffer {
public:
	RingBuffer();
	~RingBuffer();

private:
	// ���� ����
	char* begin;
	char* end;

	char* front;
	char* rear;

	char* read_pos;
	char* write_pos;

private:
	// inline ���� �ȵ�
	inline int Must_Enqueue(const void* src, size_t size);
	inline int Must_Dequeue(void* dst, size_t size);

	inline void Set_ReadPos() { read_pos = begin + MASKING_BIT((ULONG_PTR)front + 1); }
	inline void Set_WritePos() { write_pos = begin + MASKING_BIT((ULONG_PTR)rear + 1); }

public:
	inline void Move_Front(int size) { front = begin + MASKING_BIT((ULONG_PTR)front + size); Set_ReadPos();  }
	inline void Move_Rear(int size) { rear = begin + MASKING_BIT((ULONG_PTR)rear + size);    Set_WritePos(); }

	inline bool Empty() const { return front == rear; }
	inline bool Full() const { return front == write_pos; }
	inline void Clear();

	inline int Direct_EnqueueSize() const;
	inline int Direct_DequeueSize() const;
	inline int Remain_EnqueueSize() const;
	inline int Remain_DequeueSize() const;

	// inline ���� �ȵ�
	inline int Get_FreeSize() const;
	inline int Get_UseSize() const;

	inline char* Get_ReadPos() const { return read_pos; }
	inline char* Get_WritePos()  const { return write_pos; }
	inline char* Get_BeginPos() const { return begin; }

	int Enqueue(const void* src, size_t size);
	int Dequeue(void* dst, size_t size);
	int Peek(void* dst, size_t size) const;
};

inline int RingBuffer::Must_Enqueue(const void* src, size_t size) {
	memmove(write_pos, src, size);
	Move_Rear(size);
	return size;
}

inline int RingBuffer::Must_Dequeue(void* dst, size_t size) {
	memmove(dst, read_pos, size);
	Move_Front(size);
	return size;
}

inline void RingBuffer::Clear(){
	front = begin;
	rear = begin;

	write_pos = front + 1;
	read_pos = rear + 1;
}

///////////////////////////////////////////////////////////////////////
// Enqueue, Dequeue Size �� ù��° ������ ����ؾ��� 
// 
// ex. ù��° ���� ������(wp, rp) ���� �տ� �ִٸ�, end ���� ��� ��
// �ٽ� begin ���� ù��° ��(wp, rp)���� ����ؾ���
///////////////////////////////////////////////////////////////////////

inline int RingBuffer::Direct_EnqueueSize() const {
	if (front < write_pos) return end - write_pos; // ù��° �� (front) ����
	else return front - write_pos;
}

inline int RingBuffer::Remain_EnqueueSize() const {
	if (write_pos <= front) return 0; // ù��° �� ����
	else return front - begin;
}

inline int RingBuffer::Direct_DequeueSize() const {
	if (write_pos < read_pos) return end - read_pos; // ù��° ��  (write pos) ����
	else return write_pos - read_pos;
}

inline int RingBuffer::Remain_DequeueSize() const {
	if (read_pos <= write_pos) return 0; // ù��° �� ����
	else return write_pos - begin;
}

int RingBuffer::Get_FreeSize() const {
	return  Direct_EnqueueSize() + Remain_EnqueueSize();
}
inline int RingBuffer::Get_UseSize() const {
	return  Direct_DequeueSize() + Remain_DequeueSize();
};

inline static int Must_Peek(void* dst, const void* src, size_t size) {
	memmove(dst, src, size);
	return size;
}