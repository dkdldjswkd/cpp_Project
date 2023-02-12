#pragma warning(disable : 4996)

#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

#define RMOTEIP "255.255.255.255"
#define SERVER_IP	"118.222.48.48"
#define SERVER_PORT	3000
#define BUFSIZE 256

struct Protocol
{
	char data1 = 0xff;
	char data2 = 0xee;
	char data3 = 0xdd;
	char data4 = 0xaa;
	char data5 = 0x00;
	char data6 = 0x99;
	char data7 = 0x77;
	char data8 = 0x55;
	char data9 = 0x33;
	char data10 = 0x11;
	// 0xff 0xee 0xdd 0xaa 0x00 0x99 0x77 0x55 0x33 0x11 
};

int main() {

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cout << "WSAStartup() Error" << endl;
		cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
		return 0;
	}

	SOCKET send_sock = socket(AF_INET, SOCK_DGRAM, NULL);
	if (send_sock == INVALID_SOCKET) {
		cout << "socket() Error" << endl;
		cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
		return 0;
	}

	// ��ε� ĳ���� Ȱ��ȭ
	BOOL enable = TRUE;
	auto ret = setsockopt(send_sock, SOL_SOCKET, SO_BROADCAST, (char*)&enable, sizeof(enable));
	if (ret == SOCKET_ERROR) {
		cout << "setsockopt() Error" << endl;
		cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
		return 0;
	}

	// blocked �ð� �ɼ� �ɱ�
	struct timeval optVal = { 0, 200 };
	ret = setsockopt(send_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&optVal, sizeof(optVal));
	if (ret == SOCKET_ERROR) {
		cout << "setsockopt() Error" << endl;
		cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
		return 0;
	}

	char recv_buf[BUFSIZE];

	for (int i = 10001; i <= 10099; i++) {
		SOCKADDR_IN send_addr;
		ZeroMemory(&send_addr, sizeof(send_addr));
		send_addr.sin_family = AF_INET;
		send_addr.sin_addr.s_addr = inet_addr(RMOTEIP);
		send_addr.sin_port = htons(i); //  10001 ~10099 �� �˻��ؾ���

		Protocol* packet = new Protocol;

		// ��ε� ĳ����
		auto send_ret = sendto(send_sock, (const char*)packet, sizeof(Protocol), 0,
			(SOCKADDR*)&send_addr, sizeof(send_addr));
		if (SOCKET_ERROR == send_ret) {
			cout << "error sendto()" << endl;
			cout << "error no : " << WSAGetLastError() << endl;
		}

		// recv �غ�
		SOCKADDR_IN server_addr;
		int addr_len = sizeof(server_addr);


		auto recv_ret = recvfrom(send_sock, recv_buf, BUFSIZE, 0,
			(sockaddr*)&server_addr, &addr_len);

		if (recv_ret == SOCKET_ERROR) {
			auto erro_no = WSAGetLastError();
			if (WSAEWOULDBLOCK == erro_no)
			{
				//cout << "200ms timeout" << endl;
				break;
			}
		}
		else {
			cout << i << "�� ��Ʈ ������� recv !!" << endl;
			wchar_t w_buf[100];
			memcpy(w_buf, recv_buf, 100);
			w_buf[recv_ret / 2] = L'\0';

			cout << "ip : " << inet_ntoa(server_addr.sin_addr) << endl;
			//cout << "port : " <<i << endl;
			cout << "port : " << ntohs(server_addr.sin_port) << endl;
			wprintf(L"message : %s \n", w_buf);
			cout << "message len : " << wcslen(w_buf) << endl;

			cout << endl;
		}



	}

	WSACleanup();



	//while (1)
	//{
	//	QueryPerformanceCounter(&Start);

	//	QueryPerformanceCounter(&End);
	//	double t = (End.QuadPart - Start.QuadPart) / (double)(Freq.QuadPart / 1000); // ms
	//	auto sleep_time = 50 - t;

	//	//cout << "frame : " << frame++ << " / frame_time : / " << frame_time << " / t : " << t << endl;
	//}
}

//# ��ε� ĳ������ ���ؼ� ���� ��Ʈ�� ���� ��ٸ��� ���ӹ� �˻�
//
//�������� - 0xff 0xee 0xdd 0xaa 0x00 0x99 0x77 0x55 0x33 0x11   (10 Byte)
//
//��Ʈ���� - 10001 ~10099
//
/////////////////////////////////////////////////////////////
//
//Ư�� ��Ʈ�� ���� �� �޽����� ��ε�ĳ�������� �Ѹ���.UDP
//
//�ش� ��Ʈ�� ������ �����ִٸ� ����(���� �� �̸� / UTF - 16) ���ڿ��� �ǵ��� �´�.
//
//���� ��Ʈ��ũ ���ο� �ش� ��Ʈ�� ������ ���ٸ� ������ ���� ����.
//
//
//���� 1. ��Ʈ���� 10001~10099 ������ ��� �˻��ؾ� �Ѵ�.
//
//���� 2. ������ ��������� 200�и������ ����Ѵ�. (����ũ�μ�����, �и������� ���� Ȯ��)
//
//���� 3. ������ �� �̸�, IP, Port ��ȣ�� ��� �Ѵ�.
//
//���� 4. ���� �� 10�� ��������. 10���� ��� ã�Ƴ��� ����
//
//= ��ε�ĳ������ ���� ��Ʈ��ũ ���ο��� ������ �ǹǷ� �������� �׽�Ʈ �Ұ�
//
//
/////////////////////////////////////////////////////////////
//
//# �� ���ִ� TIP
//
//1. UDP ���� ����
//2. ��ε�ĳ���� Ȱ��ȭ(���Ͽɼ� : å����, �ɼ����� : MSDN ����)
//3. ��ε�ĳ���� �� IP �� INADDR_BROADCAST / 255.255.255.255
//4. �������ݷ� ������ UDP ���
//5. 200�и������� ���� ������ ���� Ȯ��
//6. ������ ���� �ش� �۽��� Ȯ��, ������ Ȯ�� �� IP, Port, ���̸� ���
//
//
//�� ������� ������ ��Ʈ ������ ��� ã�ƺ�.