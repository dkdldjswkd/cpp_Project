#include "PacketBuffer.h"
#include "NetServer.h"

LFObjectPoolTLS<PacketBuffer>  PacketBuffer::packetPool;

PacketBuffer::PacketBuffer() : buf_size(HEADER_SPACE + PAYLOAD_SPACE) {
	begin = (char*)malloc(buf_size);
	end = begin + buf_size;

	payload_pos = begin + HEADER_SPACE;
	write_pos = begin + HEADER_SPACE;
}

void PacketBuffer::Set() {
	payload_pos = begin + HEADER_SPACE;
	write_pos = begin + HEADER_SPACE;
	encrypt_flag = false;
	ref_count = 1;
}

PacketBuffer* PacketBuffer::Alloc() {
	PacketBuffer* p_packet = packetPool.Alloc();
	p_packet->Set();
	return p_packet;
}

int PacketBuffer::Free(PacketBuffer* instance) {
	auto ref_count = InterlockedDecrement((DWORD*)&instance->ref_count);
	if (ref_count == 0) {
		packetPool.Free(instance);
	}
	return ref_count;
}

void PacketBuffer::SetLanHeader() {
	LAN_HEADER lanHeader;
	lanHeader.len = GetPayloadSize();
	memmove(GetLanPacketPos(), &lanHeader, LAN_HEADER_SIZE);
}

void PacketBuffer::SetNetHeader(BYTE protocol_code, BYTE private_key) {
	// 중복 암호화 하지 않기 위함 (이미 암호화 된 패킷)
	if (encrypt_flag) return;
	encrypt_flag = true;

	NET_HEADER netHeader;
	netHeader.code = protocol_code;
	netHeader.len = GetPayloadSize();
	netHeader.randKey = (rand() & 0xFF);
	netHeader.checkSum = GetChecksum();
	memmove(GetNetPacketPos(), &netHeader, NET_HEADER_SIZE);

	char* encryptPos = payload_pos - 1;		// 암호화'될' 주소
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

// this에 암호 패킷 복호화 작업
bool PacketBuffer::DecryptPacket(char* encryptPacket, BYTE private_key) {
	char* const packetPos			= GetNetPacketPos();					// 복호화'될' 패킷 시작 주소
	char* const packetPos_encrypt	= encryptPacket;						// 암호패킷 시작 주소
	char* decryptPos = packetPos + (NET_HEADER_SIZE - 1);					// 복호화'될' 주소
	char* encryptPos = packetPos_encrypt + (NET_HEADER_SIZE - 1);			// 암호 주소
	const short encrypt_len = ((NET_HEADER*)packetPos_encrypt)->len + 1;	// 암호 길이
	BYTE RK = ((NET_HEADER*)packetPos_encrypt)->randKey;	// 랜덤 키
	BYTE K = private_key;										// 고정 키
	BYTE P = 0, LP = 0, LE = 0;								// 복호화 변수

	// 복호화
	Move_Wp(encrypt_len - 1); // 복호화 되여 복사되는 페이로드 부 만큼 move writePos
	memmove(packetPos, packetPos_encrypt, NET_HEADER_SIZE - 1); // 암호화 안된부분 복사
	for (int i = 0; i < encrypt_len; i++, encryptPos++, decryptPos++) {
		P = (*encryptPos) ^ (LE + K + (i + 1));
		*((BYTE*)decryptPos) = P ^ (LP + RK + (i + 1)); // BYTE 단위 복호화
		// 다음 복호화 루프를 위해 Set
		LE = *encryptPos;
		LP = P;
	}

	// 패킷 유효 여부 검사 (false 시 유효하지 않은 패킷(or 복호화 로직 결함))
	if (((NET_HEADER*)packetPos)->checkSum == GetChecksum()) {
		return true;
	}
	return false;
}

BYTE PacketBuffer::GetChecksum() {
	WORD len = GetPayloadSize();

	DWORD checkSum = 0;
	char* cpy_readPos = payload_pos;
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
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned char& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const int& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned int& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const long& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned long& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const short& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned short& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const float& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const double& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const long long& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned long long& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const char* str) {
	int size = strlen(str);

	if (write_pos + size <= end) {
		memmove(write_pos, str, size);
		write_pos += size;
	}
	else {
		throw PacketException(PUT_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const wchar_t* str) {
	int size = wcslen(str) * 2;

	if (write_pos + size <= end) {
		memmove(write_pos, str, size);
		write_pos += size;
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
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned char& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(int& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned int& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(long& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned long& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(short& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned short& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(float& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}

	return *this;
}

PacketBuffer& PacketBuffer::operator>>(double& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(long long& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned long long& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw PacketException(GET_ERROR);
	}
	return *this;
}

///////////////////////////////
//	PUT, GET
///////////////////////////////

void PacketBuffer::Put_Data(const char* src, int size) {
	if (write_pos + size <= end) {
		memmove(write_pos, src, size);
		write_pos += size;
	}
	else {
		throw PacketException(PUT_ERROR);
	}
}

void PacketBuffer::Get_Data(char* dst, int size) {
	if (payload_pos + size <= write_pos) {
		memmove(dst, payload_pos, size);
		payload_pos += size;
	}
	else {
		throw PacketException(GET_ERROR);
	}
}