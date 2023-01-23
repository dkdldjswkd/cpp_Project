#pragma once
#include <memory>
#include <mutex>
#include <Windows.h>
#include <winnt.h>
#include "LFObjectPoolTLS.h"
#include "protocol.h"

constexpr unsigned PAYLOAD_SPACE = 8000;
class NetworkLib;

namespace J_LIB {
class PacketBuffer {
private:
	PacketBuffer();
public:
	inline ~PacketBuffer();

private:
	friend NetworkLib;
	friend LFObjectPool<PacketBuffer>;

private:
	static LFObjectPool<PacketBuffer> packetPool; 

private:
	char* begin;
	char* end;
	const int buf_size;
	int ref_count;
	bool encrypt_flag = false;

public:
	char* write_pos;
	char* payload_pos;

public:
	static PacketBuffer* Alloc(); // 밖에서 편하게 사용할 수 있게 해야함
	static PacketBuffer* Alloc_LanPacket();
	static PacketBuffer* Alloc_NetPacket();
	static int Free(PacketBuffer* instance);

private:
	// 네트워크 라이브러리에서 사용하기 위함
	void Set_LanHeader();
	void Set_NetHeader();
	bool DecryptPacket(PacketBuffer* encryptPacket);
	char* GetPacketPos_LAN(); // 네트워크 헤더 시작위치 반환
	char* GetPacketPos_NET();
	static int GetUseCount();
	inline int Get_PacketSize_LAN();
	inline int Get_PacketSize_NET();

private:
	BYTE Get_CheckSum();

public:
	inline void Set_Lan();
	inline void Set_Net();
	inline bool Empty() const;
	inline bool Full() const;
	inline int Get_FreeSize()const;
	inline int Get_PayloadSize() const;
	inline int Get_BufSize() const;
	inline char* Get_writePos() const;
	inline char* Get_payloadPos() const;
	void Increment_refCount();

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
	inline void Move_Rp(int size) { payload_pos += size; }
};

inline PacketBuffer::~PacketBuffer() {
	free(begin);
}

inline PacketBuffer* PacketBuffer::Alloc_LanPacket(){
	PacketBuffer* p = packetPool.Alloc();
	p->Set_Lan();
	return p;
}

inline PacketBuffer* PacketBuffer::Alloc_NetPacket(){
	PacketBuffer* p = packetPool.Alloc();
	p->Set_Net();
	return p;
}

inline int PacketBuffer::Free(PacketBuffer* instance){
	auto ref_count = InterlockedDecrement((DWORD*)&instance->ref_count);
	if (ref_count == 0) {
		packetPool.Free(instance);
	}

	return ref_count;
}

inline int PacketBuffer::GetUseCount() {
	//return packetPool.Get_UseCount();
	return packetPool.GetUseCount();
}

inline int PacketBuffer::Get_PacketSize_LAN() {
	return (write_pos - payload_pos) + LAN_HEADER_SIZE;
}

inline int PacketBuffer::Get_PacketSize_NET() {
	return (write_pos - payload_pos) + NET_HEADER_SIZE;
}

inline void PacketBuffer::Increment_refCount(){
	InterlockedIncrement((DWORD*)&ref_count);
}

inline BYTE PacketBuffer::Get_CheckSum() {
	WORD len = Get_PayloadSize();

	DWORD checkSum = 0;
	char* cpy_readPos = payload_pos;
	for (int i = 0; i < len; i++) {
		checkSum += *cpy_readPos;
		cpy_readPos++;
	}
	//return (BYTE)(checkSum & 0xFF);
	return (BYTE)(checkSum % 256);
}

inline char* PacketBuffer::GetPacketPos_LAN(){
	return (payload_pos - LAN_HEADER_SIZE);
}

inline char* PacketBuffer::GetPacketPos_NET(){
	return (payload_pos - NET_HEADER_SIZE);
}

inline void PacketBuffer::Set_Lan() {
	write_pos = begin + LAN_HEADER_SIZE;
	payload_pos = begin + LAN_HEADER_SIZE;
	ref_count = 1;
}

inline void PacketBuffer::Set_Net(){
	write_pos = begin + NET_HEADER_SIZE;
	payload_pos = begin + NET_HEADER_SIZE;
	encrypt_flag = false;
	ref_count = 1;
}

inline bool PacketBuffer::Empty() const {
	if (write_pos <= payload_pos)
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

inline int PacketBuffer::Get_PayloadSize() const {
	return write_pos - payload_pos;
}

inline int PacketBuffer::Get_BufSize() const {
	return buf_size;
}

inline char* PacketBuffer::Get_writePos() const {
	return write_pos;
}

inline char* PacketBuffer::Get_payloadPos() const {
	return payload_pos;
}
}