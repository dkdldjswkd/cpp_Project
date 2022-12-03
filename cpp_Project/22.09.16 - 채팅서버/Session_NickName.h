#pragma once
#include <set>

#define NICK_MAX_LEN	15

struct NickName {
	NickName(const wchar_t* src) {
		Set(src);
	}
	NickName() {}
	~NickName() {}

public:
	wchar_t nick_name[NICK_MAX_LEN] = L"";

public:
	inline void Set(const wchar_t* src) {
		wcsncpy_s(nick_name, src, NICK_MAX_LEN - 1);
	}

	NickName& operator=(const NickName& other) {
		Set((const wchar_t*)&other);
		return *this;
	}
};

struct NickName_Compare {
	bool operator() (NickName* const a, NickName* const b) const {
		return wcscmp(a->nick_name, b->nick_name);
	}
};

