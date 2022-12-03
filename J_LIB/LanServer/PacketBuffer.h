#pragma once
#include <memory>
#include <mutex>
#include <Windows.h>
#include <winnt.h>
#include "ObjectPool.h"
#include "protocol.h"

// * HEADER_SIZE 정의 필요 (구현부에서 해당 변수 이름 사용)
constexpr unsigned PAYLOAD_SPACE = 8000;

// 네트워크 라이브러리 클래스
class LanServer;

namespace J_LIB {
class PacketBuffer {
private:
	PacketBuffer();
public:
	inline ~PacketBuffer();

private:
	friend LanServer;
	friend Pool_Node<PacketBuffer>;
	friend ObjectPool<PacketBuffer>;

private:
	static ObjectPool<PacketBuffer> pool; 
	static std::mutex pool_lock;

private:
	char* begin;
	char* end;
	const int buf_size;
	int* ref_count;

public:
	char* write_pos;
	char* read_pos;

public:
	static PacketBuffer* Alloc();
	static void Free(PacketBuffer* instance);

private:
	// 네트워크 라이브러리에서 사용하기 위함
	void Set_header(void* src);
	char* Get_Packet();
	static int GetUseCount();
	inline int Get_PacketSize();
	void Increment_refCount();

public:
	inline void Clear();
	inline bool Empty() const;
	inline bool Full() const;
	inline int Get_FreeSize()const;
	inline int Get_UseSize() const;
	inline int Get_BufSize() const;
	inline char* Get_writePos() const;
	inline char* Get_readPos() const;

public:
	// instream
	PacketBuffer& operator <<(const char& data);
	PacketBuffer& operator <<(const unsigned char& data);
	PacketBuffer& operator <<(const int& data);
	PacketBuffer& operator <<(const unsigned int& data);
	PacketBuffer& operator <<(const long& data);
	PacketBuffer& operator <<(const unsigned long& data);
	PacketBuffer& operator <<(const short& data);
	PacketBuffer& operator <<(const unsigned short& data);
	PacketBuffer& operator <<(const float& data);
	PacketBuffer& operator <<(const double& data);
	PacketBuffer& operator <<(const long long& data);
	PacketBuffer& operator <<(const unsigned long long& data);

	PacketBuffer& operator <<(const char* str);
	PacketBuffer& operator <<(const wchar_t* str);

	// outstream
	PacketBuffer& operator >>(char& data);
	PacketBuffer& operator >>(unsigned char& data);
	PacketBuffer& operator >>(int& data);
	PacketBuffer& operator >>(unsigned int& data);
	PacketBuffer& operator >>(long& data);
	PacketBuffer& operator >>(unsigned long& data);
	PacketBuffer& operator >>(short& data);
	PacketBuffer& operator >>(unsigned short& data);
	PacketBuffer& operator >>(float& data);
	PacketBuffer& operator >>(double& data);
	PacketBuffer& operator >>(long long& data);
	PacketBuffer& operator >>(unsigned long long& data);

	void Put_Data(const char* src, int size);
	void Get_Data(char* dst, int size);

	inline void Move_Wp(int size) { write_pos += size; }
	inline void Move_Rp(int size) { read_pos += size; }
};

inline PacketBuffer::~PacketBuffer() {
	free(begin);
}

inline PacketBuffer* PacketBuffer::Alloc(){
	pool_lock.lock();
	PacketBuffer* p = pool.Alloc();
	pool_lock.unlock();

	p->Clear();
	p->ref_count = new int(1);
	return p;
}

inline void PacketBuffer::Free(PacketBuffer* instance){
	if (InterlockedDecrement((DWORD*)instance->ref_count) == 0) {
		pool_lock.lock();
		pool.Free(instance);
		pool_lock.unlock();
	}
}

inline int PacketBuffer::GetUseCount() {
	return pool.GetUseCount();
}

inline int PacketBuffer::Get_PacketSize() {
	return write_pos - (read_pos - HEADER_SIZE);
}

inline void PacketBuffer::Increment_refCount(){
	InterlockedIncrement((DWORD*)this->ref_count);
}

inline void PacketBuffer::Set_header(void* src){
	memmove(read_pos - HEADER_SIZE, src, HEADER_SIZE);
}

inline char* PacketBuffer::Get_Packet(){
	return (read_pos - HEADER_SIZE);
}

inline void PacketBuffer::Clear() {
	write_pos = begin + HEADER_SIZE;
	read_pos = begin + HEADER_SIZE;
}

inline bool PacketBuffer::Empty() const {
	if (write_pos <= read_pos)
		return true;
	return false;
}

bool PacketBuffer::Full() const {
	if (write_pos + 1 == end)
		return true;
	return false;
}

inline int PacketBuffer::Get_FreeSize() const {
	return end - write_pos;
}

inline int PacketBuffer::Get_UseSize() const {
	return write_pos - read_pos;
}

inline int PacketBuffer::Get_BufSize() const {
	return buf_size;
}

inline char* PacketBuffer::Get_writePos() const {
	return write_pos;
}

inline char* PacketBuffer::Get_readPos() const {
	return read_pos;
}

}