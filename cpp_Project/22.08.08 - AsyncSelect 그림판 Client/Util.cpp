#include "stdafx.h"
#include "Util.h"

int Check_SocketError(bool error) {
	if (error) {
		auto error_num = WSAGetLastError();

		switch (error_num)
		{
		case WSAECONNRESET:
			// ����ó��
			return 10054;

		default:
			break;
		}
		
		CRASH();
	}
}