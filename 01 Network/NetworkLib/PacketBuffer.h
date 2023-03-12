#pragma once
#include <memory>
#include <mutex>
#include <Windows.h>
#include <winnt.h>
#include "../../00 lib_jy/LFObjectPoolTLS.h"
#include "protocol.h"
#include <exception>

#define MAX_PAYLOAD_LEN 8000
#define MAX_HEADER_LEN 10

struct PacketException : public std::exception {
public:
	PacketException(const char* error_code, bool type) : exception(error_code), errorType(type) {}
	PacketException(bool type) : errorType(type) {}

public:
	bool errorType;
#define GET_ERROR 0 // >> Error
#define PUT_ERROR 1 // << Error
};

class NetServer;
class NetClient;
class PacketBuffer {
private:
	PacketBuffer();
public:
	inline ~PacketBuffer();

private:
	friend NetServer;
	friend NetClient;
	friend LFObjectPoolTLS<PacketBuffer>;

private:
	static LFObjectPoolTLS<PacketBuffer> packetPool;

private:
	char* begin;
	char* end;
	const int buf_size;
	int ref_count;
	bool encrypt_flag;
	char* write_pos;
	char* payload_pos;

private:
	// 네트워크 라이브러리에서 사용
	void SetLanHeader();
	void SetNetHeader(BYTE protocol_code, BYTE private_key);
	bool DecryptPacket(char* encryptPacket, BYTE private_key);
	char* GetLanPacketPos();
	char* GetNetPacketPos();
	inline int GetLanPacketSize();
	inline int GetNetPacketSize();

private:
	BYTE GetChecksum();
	void Set();

public:
	static PacketBuffer* Alloc();
	static int Free(PacketBuffer* instance);

public:
	inline bool Empty() const;
	inline bool Full() const;
	inline int GetFreeSize()const;
	inline int GetPayloadSize() const;
	inline int GetBufSize() const;
	inline char* GetWritePos() const;
	inline char* GetPayloadPos() const;
	void IncrementRefCount();
	static int GetUseCount();

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

//////////////////////////////
// 구현 부
//////////////////////////////

inline PacketBuffer::~PacketBuffer() {
	free(begin);
}

inline int PacketBuffer::GetUseCount() {
	return packetPool.GetUseCount();
}

inline int PacketBuffer::GetLanPacketSize() {
	return (write_pos - payload_pos) + LAN_HEADER_SIZE;
}

inline int PacketBuffer::GetNetPacketSize() {
	return (write_pos - payload_pos) + NET_HEADER_SIZE;
}

inline void PacketBuffer::IncrementRefCount() {
	InterlockedIncrement((DWORD*)&ref_count);
}

inline char* PacketBuffer::GetLanPacketPos() {
	return (payload_pos - LAN_HEADER_SIZE);
}

inline char* PacketBuffer::GetNetPacketPos() {
	return (payload_pos - NET_HEADER_SIZE);
}

inline bool PacketBuffer::Empty() const {
	if (write_pos <= payload_pos) return true;
	return false;
}

bool PacketBuffer::Full() const {
	if (write_pos + 1 == end) return true;
	return false;
}

inline int PacketBuffer::GetFreeSize() const {
	return end - write_pos;
}

inline int PacketBuffer::GetPayloadSize() const {
	return write_pos - payload_pos;
}

inline int PacketBuffer::GetBufSize() const {
	return buf_size;
}

inline char* PacketBuffer::GetWritePos() const {
	return write_pos;
}

inline char* PacketBuffer::GetPayloadPos() const {
	return payload_pos;
}