#pragma once
#include <list>

#define MAX_ROOM_NAME 256
#define MAX_ROOM_SESSION 5

struct ChatRoom {
	int room_no;
	wchar_t room_name[MAX_ROOM_NAME];
	std::list<int> session_list;

	void Set(int no, const wchar_t *wcsstr) {
		room_no = no;
		wcsncpy_s(room_name, wcsstr, MAX_ROOM_NAME);
	}
};

// 0번은 없는 방 번호
inline unsigned short Get_ChatRoom_No() {
	static unsigned short id = 1;
	return id++;
}