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
	// �Ҵ� ����ȭ ����
	char* begin;
	char* end;
	const int bufSize;

	// �ܺ� read/write (����ȭ, ������ȭ)
	char* payloadPos;
	char* writePos;

	// ����
	int refCount;
	bool encrypted;

private:
	// ��Ŷ �ʱ�ȭ
	void Set();

	// ��� ����
	void SetLanHeader();
	void SetNetHeader(BYTE protocolCode, BYTE privateKey);

	// ��Ŷ ��ȣȭ
	bool DecryptPacket(char* encryptPacket, BYTE privateKey);

	// ��Ŷ ���� �ּ� ��ȯ (payLoadPos - headerLen)
	char* GetLanPacketPos();
	char* GetNetPacketPos();

	// ��Ŷ ������ ��ȯ (��Ŷ ��� ������ ����)
	inline int GetLanPacketSize();
	inline int GetNetPacketSize();

	// üũ�� ��ȯ
	BYTE GetChecksum();

public:
	// �Ҵ�, ��ȯ
	static PacketBuffer* Alloc();
	static void Free(PacketBuffer* instance);
	static void Free(PacketBuffer* instance, bool* isReleased);

	// Empty & Full
	inline bool Empty() const;
	inline bool Full() const;

	// Getter
	inline int GetFreeSize()const;
	inline int GetPayloadSize() const;

	// ref ����
	void IncrementRefCount();

	// Pool Count ��ȯ
	static int GetUseCount();

	// move pos
	inline void MoveWp(int size) { writePos += size; }
	inline void MoveRp(int size) { payloadPos += size; }

	// ����ȭ
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
	void PutData(const char* src, int size);

	// ������ȭ
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
	void GetData(char* dst, int size);
};

inline PacketBuffer::~PacketBuffer() {
	free((char*)begin);
}

inline int PacketBuffer::GetUseCount() {
	return packetPool.GetUseCount();
}

inline int PacketBuffer::GetLanPacketSize() {
	return (writePos - payloadPos) + LAN_HEADER_SIZE;
}

inline int PacketBuffer::GetNetPacketSize() {
	return (writePos - payloadPos) + NET_HEADER_SIZE;
}

inline void PacketBuffer::IncrementRefCount() {
	InterlockedIncrement((DWORD*)&refCount);
}

inline char* PacketBuffer::GetLanPacketPos() {
	return (payloadPos - LAN_HEADER_SIZE);
}

inline char* PacketBuffer::GetNetPacketPos() {
	return (payloadPos - NET_HEADER_SIZE);
}

inline bool PacketBuffer::Empty() const {
	if (writePos <= payloadPos) return true;
	return false;
}

bool PacketBuffer::Full() const {
	if (writePos + 1 == end) return true;
	return false;
}

inline int PacketBuffer::GetFreeSize() const {
	return end - writePos;
}

inline int PacketBuffer::GetPayloadSize() const {
	return writePos - payloadPos;
}