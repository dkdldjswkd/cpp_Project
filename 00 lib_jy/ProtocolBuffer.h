#pragma once
#include <memory>

#define BUF_SIZE 4000

class ProtocolBuffer {
public:
	ProtocolBuffer(int size = BUF_SIZE);
	inline ~ProtocolBuffer();

private:
	char* begin;
	char* end;
	const int buf_size;

public:
	char* write_pos;
	char* read_pos;

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
	ProtocolBuffer& operator <<(const char& data);
	ProtocolBuffer& operator <<(const unsigned char& data);
	ProtocolBuffer& operator <<(const int& data);
	ProtocolBuffer& operator <<(const unsigned int& data);
	ProtocolBuffer& operator <<(const long& data);
	ProtocolBuffer& operator <<(const unsigned long& data);
	ProtocolBuffer& operator <<(const short& data);
	ProtocolBuffer& operator <<(const unsigned short& data);
	ProtocolBuffer& operator <<(const float& data);
	ProtocolBuffer& operator <<(const double& data);
	ProtocolBuffer& operator <<(const long long& data);
	ProtocolBuffer& operator <<(const unsigned long long& data);

	ProtocolBuffer& operator <<(const char* str);
	ProtocolBuffer& operator <<(const wchar_t* str);

	// outstream
	ProtocolBuffer& operator >>(char& data);
	ProtocolBuffer& operator >>(unsigned char& data);
	ProtocolBuffer& operator >>(int& data);
	ProtocolBuffer& operator >>(unsigned int& data);
	ProtocolBuffer& operator >>(long& data);
	ProtocolBuffer& operator >>(unsigned long& data);
	ProtocolBuffer& operator >>(short& data);
	ProtocolBuffer& operator >>(unsigned short& data);
	ProtocolBuffer& operator >>(float& data);
	ProtocolBuffer& operator >>(double& data);
	ProtocolBuffer& operator >>(long long& data);
	ProtocolBuffer& operator >>(unsigned long long& data);

	void Put_Data(const char* src, int size);
	void Get_Data(char* dst, int size);

	inline void Move_Wp(int size) { write_pos += size; }
	inline void Move_Rp(int size) { read_pos += size; }
};

inline ProtocolBuffer::~ProtocolBuffer() {
	free(begin);
}

inline void ProtocolBuffer::Clear() {
	write_pos = begin;
	read_pos = begin;
}

inline bool ProtocolBuffer::Empty() const {
	if (write_pos <= read_pos)
		return true;
	return false;
}

bool ProtocolBuffer::Full() const {
	if (write_pos + 1 == end)
		return true;
	return false;
}

inline int ProtocolBuffer::Get_FreeSize() const {
	return end - write_pos;
}

inline int ProtocolBuffer::Get_UseSize() const {
	return write_pos - read_pos;
}

inline int ProtocolBuffer::Get_BufSize() const {
	return buf_size;
}

inline char* ProtocolBuffer::Get_writePos() const {
	return write_pos;
}

inline char* ProtocolBuffer::Get_readPos() const {
	return read_pos;
}