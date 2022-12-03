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

	// 브로드 캐스팅 활성화
	BOOL enable = TRUE;
	auto ret = setsockopt(send_sock, SOL_SOCKET, SO_BROADCAST, (char*)&enable, sizeof(enable));
	if (ret == SOCKET_ERROR) {
		cout << "setsockopt() Error" << endl;
		cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
		return 0;
	}

	// blocked 시간 옵션 걸기
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
		send_addr.sin_port = htons(i); //  10001 ~10099 를 검색해야함

		Protocol* packet = new Protocol;

		// 브로드 캐스팅
		auto send_ret = sendto(send_sock, (const char*)packet, sizeof(Protocol), 0,
			(SOCKADDR*)&send_addr, sizeof(send_addr));
		if (SOCKET_ERROR == send_ret) {
			cout << "error sendto()" << endl;
			cout << "error no : " << WSAGetLastError() << endl;
		}

		// recv 준비
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
			cout << i << "번 포트 대상으로 recv !!" << endl;
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

//# 브로드 캐스팅을 통해서 현재 포트를 열고 기다리는 게임방 검색
//
//프로토콜 - 0xff 0xee 0xdd 0xaa 0x00 0x99 0x77 0x55 0x33 0x11   (10 Byte)
//
//포트범위 - 10001 ~10099
//
/////////////////////////////////////////////////////////////
//
//특정 포트에 대해 위 메시지를 브로드캐스팅으로 뿌린다.UDP
//
//해당 포트에 서버가 열려있다면 응답(게임 방 이름 / UTF - 16) 문자열이 되돌아 온다.
//
//로컬 네트워크 내부에 해당 포트의 서버가 없다면 응답이 오지 않음.
//
//
//조건 1. 포트범위 10001~10099 까지를 모두 검색해야 한다.
//
//조건 2. 데이터 응답까지는 200밀리세컨즈를 대기한다. (마이크로세컨즈, 밀리세컨드 필히 확인)
//
//조건 3. 서버의 방 이름, IP, Port 번호를 출력 한다.
//
//조건 4. 방은 총 10개 열려있음. 10개를 모두 찾아내면 성공
//
//= 브로드캐스팅은 같은 네트워크 내부에만 전송이 되므로 집에서는 테스트 불가
//
//
/////////////////////////////////////////////////////////////
//
//# 다 퍼주는 TIP
//
//1. UDP 소켓 생성
//2. 브로드캐스팅 활성화(소켓옵션 : 책참고, 옵션인자 : MSDN 참고)
//3. 브로드캐스팅 시 IP 는 INADDR_BROADCAST / 255.255.255.255
//4. 프로토콜로 데이터 UDP 쏘기
//5. 200밀리세컨드 동안 데이터 수신 확인
//6. 데이터 오면 해당 송신자 확인, 데이터 확인 후 IP, Port, 방이름 출력
//
//
//위 방법으로 정해진 포트 범위를 모두 찾아봄.