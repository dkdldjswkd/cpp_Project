#include <iostream>
#include <Windows.h>
#include <synchapi.h>
#include <thread>
#include <stack>
#include <mutex>

using namespace std;

//char payload[256] = "aaaaaaaaaabbbbbbbbbbcccccccccc1234567890abcdefghijklmn"; // ������ 55
//
//void f() {
//	printf("��ȣȭ -- \n");
//
//	char* encryptPos = payload; // ��ȣȭ'��' �ּ�
//	short encrypt_len = 55;		// ��ȣȭ�� ����
//	BYTE RK = 0x31; // ���� Ű
//	BYTE K = 0xa9;	// ���� Ű
//	BYTE P = 0, E = 0;
//	for (int i = 0; i < encrypt_len; i++, encryptPos++) {
//		P = (*encryptPos) ^ (P + RK + (i + 1));
//		E = P ^ (E + K + (i + 1));
//		*((BYTE*)encryptPos) = E; // ��ȣȭ
//
//		printf("%X ", (BYTE) * encryptPos);
//	}
//	printf("\n");
//}
//
//char decrypt[256] = { 0, };
//void f2() {
//	printf("��ȣȭ -- \n");
//
//	char* encryptPos = payload; // ��ȣ �ּ�
//	short encrypt_len = 55;		// ��ȣ ����
//	char* decryptPos = decrypt; // ��ȣȭ'��' �ּ�
//	BYTE RK = 0x31; // ���� Ű
//	BYTE K = 0xa9;	// ���� Ű
//	BYTE P = 0, LP = 0, LE = 0;
//	for (int i = 0; i < encrypt_len; i++, encryptPos++) {
//		P					 = (*encryptPos) ^ (LE  + K + (i + 1));
//		*((BYTE*)decryptPos) = P			 ^ (LP + RK + (i + 1)); // ��ȣȭ
//
//		LE = *encryptPos;
//		LP = P;
//		printf("%X ", *decryptPos);
//	}
//}

char buf[3] = "";
char tmp[100] = "999999";

int main() {
#pragma warning(suppress : 4996)
	strncpy(buf, tmp, 4);
	cout << buf << endl;
}