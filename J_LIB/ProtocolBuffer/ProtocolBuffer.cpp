#include "ProtocolBuffer.h"

ProtocolBuffer::ProtocolBuffer(int size) : buf_size(size){
	begin = (char*)malloc(size);
	end = begin + size;

	write_pos = begin;
	read_pos = begin;
}

///////////////////////////////
//	operator <<
///////////////////////////////

ProtocolBuffer& ProtocolBuffer::operator<<(const char& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator<<(const unsigned char& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator<<(const int& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator<<(const unsigned int& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator<<(const long& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator<<(const unsigned long& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator<<(const short& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator<<(const unsigned short& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator<<(const float& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator<<(const double& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator<<(const long long& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator<<(const unsigned long long& data) {
	if (write_pos + sizeof(data) <= end) {
		memmove(write_pos, &data, sizeof(data));
		write_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator<<(const char* str) {
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

ProtocolBuffer& ProtocolBuffer::operator<<(const wchar_t* str) {
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

ProtocolBuffer& ProtocolBuffer::operator>>(char& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator>>(unsigned char& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator>>(int& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator>>(unsigned int& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator>>(long& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator>>(unsigned long& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator>>(short& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator>>(unsigned short& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator>>(float& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}

	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator>>(double& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator>>(long long& data) {
	if (read_pos + sizeof(data) <= write_pos) {
		memmove(&data, read_pos, sizeof(data));
		read_pos += sizeof(data);
	}
	else {
		throw;
	}
	return *this;
}

ProtocolBuffer& ProtocolBuffer::operator>>(unsigned long long& data) {
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

void ProtocolBuffer::Put_Data(const char* src, int size){
	if (write_pos + size <= end) {
		memmove(write_pos, src, size);
		write_pos += size;
	}
	else {
		throw;
	}
}

void ProtocolBuffer::Get_Data(char* dst, int size) {
	if (read_pos + size <= write_pos) {
		memmove(dst, read_pos, size);
		read_pos += size;
	}
	else {
		throw;
	}
}
