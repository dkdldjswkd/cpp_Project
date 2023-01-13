#include "PacketBuffer.h"

namespace J_LIB {

LFObjectPoolTLS<PacketBuffer>  PacketBuffer::packetPool;

// header size 
PacketBuffer::PacketBuffer() : buf_size(HEADER_SIZE + PAYLOAD_SPACE) {
	begin = (char*)malloc(buf_size);
	end = begin + buf_size;

	write_pos = begin + HEADER_SIZE;
	read_pos = begin + HEADER_SIZE;
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
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned char& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(int& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned int& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(long& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned long& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(short& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned short& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(float& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}

	return *this;
}

PacketBuffer& PacketBuffer::operator>>(double& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(long long& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

PacketBuffer& PacketBuffer::operator>>(unsigned long long& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
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
	if (read_pos + size <= write_pos) {
		memmove(dst, read_pos, size);
		read_pos += size;
	}
	else {
		throw;
	}
}


}