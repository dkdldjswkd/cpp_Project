#include "PacketBuffer.h"
#include "NetServer.h"

LFObjectPoolTLS<PacketBuffer>  PacketBuffer::packetPool;

PacketBuffer::PacketBuffer() : bufSize(MAX_HEADER_LEN + MAX_PAYLOAD_LEN) {
	begin = (char*)malloc(bufSize);
	end = begin + bufSize;
	payloadPos = begin + MAX_HEADER_LEN;
	writePos = begin + MAX_HEADER_LEN;
}

void PacketBuffer::Set() {
	payloadPos = begin + MAX_HEADER_LEN;
	writePos = begin + MAX_HEADER_LEN;
	encrypted = false;
	refCount = 1;
}

PacketBuffer* PacketBuffer::Alloc() {
	PacketBuffer* p_packet = packetPool.Alloc();
	p_packet->Set();
	return p_packet;
}

void PacketBuffer::Free(PacketBuffer* instance) {
	if (0 == InterlockedDecrement((DWORD*)&instance->refCount))
		packetPool.Free(instance);
}

void PacketBuffer::Free(PacketBuffer* instance, bool* isReleased) {
	if (0 == InterlockedDecrement((DWORD*)&instance->refCount)) {
		packetPool.Free(instance);
		*isReleased = true;
	}
	*isReleased = false;
}

void PacketBuffer::SetLanHeader() {
	LanHeader lanHeader;
	lanHeader.len = GetPayloadSize();
	memmove(GetLanPacketPos(), &lanHeader, LAN_HEADER_SIZE);
}

void PacketBuffer::SetNetHeader(BYTE protocol_code, BYTE private_key) {
	// 중복 암호화 하지 않기 위함 (이미 암호화 된 패킷)
	if (encrypted) return;
	encrypted = true;

	NetHeader netHeader;
	netHeader.code = protocol_code;
	netHeader.len = GetPayloadSize();
	netHeader.randKey = (rand() & 0xFF);
	netHeader.checkSum = GetChecksum();
	memmove(GetNetPacketPos(), &netHeader, NET_HEADER_SIZE);

	char* encryptPos = payloadPos - 1;		// 암호화'될' 주소
	short encrypt_len = netHeader.len + 1;	// 암호화될 길이
	BYTE RK = netHeader.randKey;			// 랜덤 키
	BYTE K = private_key;					// 고정 키
	BYTE P = 0, E = 0;						// 암호화 변수

	// 암호화
	for (int i = 0; i < encrypt_len; i++, encryptPos++) {
		P = (*encryptPos) ^ (P + RK + (i + 1));
		E = P ^ (E + K + (i + 1));
		*((BYTE*)encryptPos) = E;
	}
}

// 암호패킷을 this에 복호화
bool PacketBuffer::DecryptPacket(char* encryptPacket, BYTE privateKey) {
	// 복호화 변수
	const BYTE RK = ((NetHeader*)encryptPacket)->randKey;
	const BYTE K = privateKey;
	BYTE P = 0, LP = 0, LE = 0;

	// 복호화 준비
	char* const packetPos = GetNetPacketPos();						// this 패킷 주소
	char* decryptPos = packetPos + (NET_HEADER_SIZE - 1);			// this 복호화'될' 주소 (복호화 부 : checksum + payload)
	char* encryptPos = encryptPacket + (NET_HEADER_SIZE - 1);		// 암호화 부
	const short encryptLen = ((NetHeader*)encryptPacket)->len + 1;	// 암호화 부 길이 (checksum + payload)
	memmove(packetPos, encryptPacket, NET_HEADER_SIZE - 1);			// 암호화 되어있지 않은 부분 복사

	// 복호화
	for (int i = 0; i < encryptLen; ++i) { // 복호화
		// BYTE 단위 복호화
		P = (*encryptPos) ^ (LE + K + (i + 1));
		*((BYTE*)decryptPos) = P ^ (LP + RK + (i + 1));

		// 다음 루프 준비
		LE = *encryptPos;
		LP = P;
		encryptPos++;
		decryptPos++;
	}
	MoveWp(encryptLen - 1); // 복호화 중 복사된 페이로드 부 만큼 move

	// 암호패킷의 신뢰성 결과 반환
	if (((NetHeader*)packetPos)->checkSum == GetChecksum()) {
		return true;
	}
	return false;
}

BYTE PacketBuffer::GetChecksum() {
	WORD len = GetPayloadSize();

	DWORD checkSum = 0;
	char* cpy_readPos = payloadPos;
	for (int i = 0; i < len; i++) {
		checkSum += *cpy_readPos;
		cpy_readPos++;
	}
	return (BYTE)(checkSum & 0xFF);
}

///////////////////////////////
//	operator <<
///////////////////////////////

PacketBuffer& PacketBuffer::operator<<(const char& data) {
	if (writePos + sizeof(data) <= end) {
		memmove(writePos, &data, sizeof(data));
		writePos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned char& data) {
	if (writePos + sizeof(data) <= end) {
		memmove(writePos, &data, sizeof(data));
		writePos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const int& data) {
	if (writePos + sizeof(data) <= end) {
		memmove(writePos, &data, sizeof(data));
		writePos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned int& data) {
	if (writePos + sizeof(data) <= end) {
		memmove(writePos, &data, sizeof(data));
		writePos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const long& data) {
	if (writePos + sizeof(data) <= end) {
		memmove(writePos, &data, sizeof(data));
		writePos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned long& data) {
	if (writePos + sizeof(data) <= end) {
		memmove(writePos, &data, sizeof(data));
		writePos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const short& data) {
	if (writePos + sizeof(data) <= end) {
		memmove(writePos, &data, sizeof(data));
		writePos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned short& data) {
	if (writePos + sizeof(data) <= end) {
		memmove(writePos, &data, sizeof(data));
		writePos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const float& data) {
	if (writePos + sizeof(data) <= end) {
		memmove(writePos, &data, sizeof(data));
		writePos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const double& data) {
	if (writePos + sizeof(data) <= end) {
		memmove(writePos, &data, sizeof(data));
		writePos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const long long& data) {
	if (writePos + sizeof(data) <= end) {
		memmove(writePos, &data, sizeof(data));
		writePos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned long long& data) {
	if (writePos + sizeof(data) <= end) {
		memmove(writePos, &data, sizeof(data));
		writePos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

///////////////////////////////
//	operator >>
///////////////////////////////

PacketBuffer& PacketBuffer::operator>>(char& data) {
	if (payloadPos + sizeof(data) <= writePos) {
		memmove(&data, payloadPos, sizeof(data));
		payloadPos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned char& data) {
	if (payloadPos + sizeof(data) <= writePos) {
		memmove(&data, payloadPos, sizeof(data));
		payloadPos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(int& data) {
	if (payloadPos + sizeof(data) <= writePos) {
		memmove(&data, payloadPos, sizeof(data));
		payloadPos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned int& data) {
	if (payloadPos + sizeof(data) <= writePos) {
		memmove(&data, payloadPos, sizeof(data));
		payloadPos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(long& data) {
	if (payloadPos + sizeof(data) <= writePos) {
		memmove(&data, payloadPos, sizeof(data));
		payloadPos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned long& data) {
	if (payloadPos + sizeof(data) <= writePos) {
		memmove(&data, payloadPos, sizeof(data));
		payloadPos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(short& data) {
	if (payloadPos + sizeof(data) <= writePos) {
		memmove(&data, payloadPos, sizeof(data));
		payloadPos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned short& data) {
	if (payloadPos + sizeof(data) <= writePos) {
		memmove(&data, payloadPos, sizeof(data));
		payloadPos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(float& data) {
	if (payloadPos + sizeof(data) <= writePos) {
		memmove(&data, payloadPos, sizeof(data));
		payloadPos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}

	return *this;
}

PacketBuffer& PacketBuffer::operator>>(double& data) {
	if (payloadPos + sizeof(data) <= writePos) {
		memmove(&data, payloadPos, sizeof(data));
		payloadPos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(long long& data) {
	if (payloadPos + sizeof(data) <= writePos) {
		memmove(&data, payloadPos, sizeof(data));
		payloadPos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned long long& data) {
	if (payloadPos + sizeof(data) <= writePos) {
		memmove(&data, payloadPos, sizeof(data));
		payloadPos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

///////////////////////////////
//	PUT, GET
///////////////////////////////

void PacketBuffer::PutData(const char* src, int size) {
	if (writePos + size <= end) {
		memmove(writePos, src, size);
		writePos += size;
	}
	else {
		throw PacketException(PUT_ERROR);
	}
}

void PacketBuffer::GetData(char* dst, int size) {
	if (payloadPos + size <= writePos) {
		memmove(dst, payloadPos, size);
		payloadPos += size;
	}
	else {
		throw PacketException(GET_ERROR);
	}
}