#pragma once
#include <memory>
#include <mutex>
#include <Windows.h>
#include <winnt.h>
#include "LFObjectPoolTLS.h"
#include "protocol.h"

constexpr unsigned PAYLOAD_SPACE = 8000;
class LanServer;

namespace J_LIB {
class PacketBuffer {
private:
	PacketBuffer();
public:
	inline ~PacketBuffer();

private:
	friend LanServer;
	friend LFObjectPoolTLS<PacketBuffer>;

private:
	static LFObjectPoolTLS<PacketBuffer> packetPool; 

private:
	char* begin;
	char* end;
	const int buf_size;
	int* ref_count;

public:
	char* write_pos;
	char* payload_pos;

public:
	static PacketBuffer* Alloc_LanPacket();
	static PacketBuffer* Alloc_NetPacket();
	static void Free(PacketBuffer* instance);

private:
	// 네트워크 라이브러리에서 사용하기 위함
	void Set_LanHeader();
	void Set_NetHeader();
	bool DecryptPacket(PacketBuffer* encryptPacket);
	char* Get_Packet(); // 패킷 시작위치 반환 (헤더 포함)
	static int GetUseCount();
	inline int Get_PacketSize();
	void Increment_refCount();

private:
	BYTE Get_CheckSum();

public:
	inline void Clear_Lan();
	inline void Clear_Net();
	inline bool Empty() const;
	inline bool Full() const;
	inline int Get_FreeSize()const;
	inline int Get_PayloadSize() const;
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
	inline void Move_Rp(int size) { payload_pos += size; }
};

inline PacketBuffer::~PacketBuffer() {
	free(begin);
}

inline PacketBuffer* PacketBuffer::Alloc_LanPacket(){
	PacketBuffer* p = packetPool.Alloc();
	p->Clear_Lan();
	p->ref_count = new int(1);
	return p;
}

inline PacketBuffer* PacketBuffer::Alloc_NetPacket(){
	PacketBuffer* p = packetPool.Alloc();
	p->Clear_Net();
	p->ref_count = new int(1);
	return p;
}

inline void PacketBuffer::Free(PacketBuffer* instance){
	if (InterlockedDecrement((DWORD*)instance->ref_count) == 0) {
		delete instance->ref_count;
		packetPool.Free(instance);
	}
}

inline int PacketBuffer::GetUseCount() {
	return packetPool.Get_UseCount();
}

inline int PacketBuffer::Get_PacketSize() {
	return write_pos - (payload_pos - LAN_HEADER_SIZE);
}

inline void PacketBuffer::Increment_refCount(){
	InterlockedIncrement((DWORD*)this->ref_count);
}

inline void PacketBuffer::Set_LanHeader(){
	LAN_HEADER lanHeader;
	lanHeader.len = Get_PayloadSize();
	memmove(begin, &lanHeader, LAN_HEADER_SIZE);
}

// Code(1byte) - Len(2byte) - RandKey(1byte) - CheckSum(1byte) - Payload(Len byte)
// 
// 사이즈: 55byte
// 
// 데이터(텍스트) : aaaaaaaaaabbbbbbbbbbcccccccccc1234567890abcdefghijklmn(널문자포함 55byte)
// 데이터(16진수) : 61 61 61 61 61 61 61 61 61 61 62 62 62 62 62 62 62 62 62 62 63 63 63 63 63 63 63 63 63 63 31 32 33 34 35 36 37 38 39 30 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 00
// 
// 고정키 : 0xa9
// 랜덤키 : 0x31
// 
// 암호화(16진수) : f9 43 95 8c 5f f3 f7 44 b1 87 46 23 ad b5 1e 01 c1 a3 1e 3f b4 80 18 1b b2 ac 36 0b 8c 9c 4a 5e 84 84 7a 0e 74 84 72 0c 16 a8 82 68 c6 ac 72 74 86 20 32 50 86 04 2d

inline BYTE PacketBuffer::Get_CheckSum() {
	WORD len = Get_PayloadSize();

	DWORD checkSum = 0;
	char* cpy_readPos = payload_pos;
	for (int i = 0; i < len; i++) {
		checkSum += *cpy_readPos;
		cpy_readPos++;
	}
	return (BYTE)(checkSum & 0xFF);
}

inline void PacketBuffer::Set_NetHeader(){
	NET_HEADER netHeader;
	netHeader.code = PROTOCOL_CODE;
	netHeader.len = Get_PayloadSize();
	netHeader.randKey = (rand() & 0xFF);
	netHeader.checkSum = Get_CheckSum();
	memmove(begin, &netHeader, NET_HEADER_SIZE);

	char* encryptPos  = payload_pos - 1;	// 암호화'될' 주소
	short encrypt_len = netHeader.len + 1;	// 암호화될 길이
	BYTE RK = netHeader.randKey; // 랜덤 키
	BYTE K  = CONST_KEY;		 // 고정 키
	BYTE P = 0, E = 0;

	// 암호화
	for (int i = 0; i < encrypt_len; i++, encryptPos++) {
		P = (*encryptPos) ^ (P + RK + (i + 1));
		E = P ^ (E + K + (i + 1));
		*((BYTE*)encryptPos) = E;
	}
}



inline char* PacketBuffer::Get_Packet(){
	return (payload_pos - LAN_HEADER_SIZE);
}

inline void PacketBuffer::Clear_Lan() {
	write_pos = begin + LAN_HEADER_SIZE;
	payload_pos = begin + LAN_HEADER_SIZE;
}

inline void PacketBuffer::Clear_Net(){
	write_pos = begin + NET_HEADER_SIZE;
	payload_pos = begin + NET_HEADER_SIZE;
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

inline char* PacketBuffer::Get_readPos() const {
	return payload_pos;
}
}