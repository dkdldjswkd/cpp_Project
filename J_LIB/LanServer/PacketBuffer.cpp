#include "PacketBuffer.h"

namespace J_LIB {

	//static LFObjectPool<PacketBuffer> packetPool;
	LFObjectPool<PacketBuffer>  PacketBuffer::packetPool;

// header size 
PacketBuffer::PacketBuffer() : buf_size(LAN_HEADER_SIZE + PAYLOAD_SPACE) {
	begin = (char*)malloc(buf_size);
	end = begin + buf_size;

	write_pos = begin + LAN_HEADER_SIZE;
	payload_pos = begin + LAN_HEADER_SIZE;
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
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned char& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const int& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned int& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const long& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned long& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const short& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned short& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const float& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const double& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const long long& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator<<(const unsigned long long& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
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
		throw;
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
		throw;
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
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned char& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(int& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned int& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(long& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned long& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(short& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned short& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(float& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw;
	}

	return *this;
}

PacketBuffer& PacketBuffer::operator>>(double& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(long long& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned long long& data) {
	if (payload_pos + sizeof(data) <= write_pos) {
		memmove(&data, payload_pos, sizeof(data));
		payload_pos += sizeof(data);
	}
	else {
		throw;
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
		throw;
	}
}

void PacketBuffer::Get_Data(char* dst, int size) {
	if (payload_pos + size <= write_pos) {
		memmove(dst, payload_pos, size);
		payload_pos += size;
	}
	else {
		throw;
	}
}

bool PacketBuffer::DecryptPacket(PacketBuffer* encryptPacket) {
	memmove(begin, encryptPacket->begin, NET_HEADER_SIZE - 1); // 암호화 안된부분 복사

	char* encryptPos = encryptPacket->begin + (NET_HEADER_SIZE - 1); // 암호 주소
	short encrypt_len = ((NET_HEADER*)encryptPacket->begin)->len;	  // 암호 길이
	char* decryptPos = begin + (NET_HEADER_SIZE - 1);				  // 복호화'될' 주소
	BYTE RK = ((NET_HEADER*)encryptPacket->begin)->randKey; // 랜덤 키
	BYTE K = CONST_KEY;										// 고정 키
	BYTE P = 0, LP = 0, LE = 0;

	// 복호화
	for (int i = 0; i < encrypt_len; i++, encryptPos++) {
		P = (*encryptPos) ^ (LE + K + (i + 1));
		*((BYTE*)decryptPos) = P ^ (LP + RK + (i + 1)); // 복호화

		LE = *encryptPos;
		LP = P;
	}

	if (((NET_HEADER*)begin)->checkSum == Get_CheckSum()) {
		return true;
	}
	return false;
}

}