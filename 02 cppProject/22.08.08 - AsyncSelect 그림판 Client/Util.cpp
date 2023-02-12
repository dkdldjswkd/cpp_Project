#include "stdafx.h"
#include "Util.h"

int Check_SocketError(bool error) {
	if (error) {
		auto error_num = WSAGetLastError();

		switch (error_num)
		{
		case WSAECONNRESET:
			// 예외처리
			return 10054;

		default:
			break;
		}
		
		CRASH();
	}
}