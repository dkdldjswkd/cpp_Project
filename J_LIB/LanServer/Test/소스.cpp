#include <iostream>
#include <Windows.h>
#include <synchapi.h>
#include <thread>
#include <stack>
#include <mutex>

using namespace std;

//char payload[256] = "aaaaaaaaaabbbbbbbbbbcccccccccc1234567890abcdefghijklmn"; // 널포함 55
//
//void f() {
//	printf("암호화 -- \n");
//
//	char* encryptPos = payload; // 암호화'될' 주소
//	short encrypt_len = 55;		// 암호화될 길이
//	BYTE RK = 0x31; // 랜덤 키
//	BYTE K = 0xa9;	// 고정 키
//	BYTE P = 0, E = 0;
//	for (int i = 0; i < encrypt_len; i++, encryptPos++) {
//		P = (*encryptPos) ^ (P + RK + (i + 1));
//		E = P ^ (E + K + (i + 1));
//		*((BYTE*)encryptPos) = E; // 암호화
//
//		printf("%X ", (BYTE) * encryptPos);
//	}
//	printf("\n");
//}
//
//char decrypt[256] = { 0, };
//void f2() {
//	printf("복호화 -- \n");
//
//	char* encryptPos = payload; // 암호 주소
//	short encrypt_len = 55;		// 암호 길이
//	char* decryptPos = decrypt; // 복호화'될' 주소
//	BYTE RK = 0x31; // 랜덤 키
//	BYTE K = 0xa9;	// 고정 키
//	BYTE P = 0, LP = 0, LE = 0;
//	for (int i = 0; i < encrypt_len; i++, encryptPos++) {
//		P					 = (*encryptPos) ^ (LE  + K + (i + 1));
//		*((BYTE*)decryptPos) = P			 ^ (LP + RK + (i + 1)); // 복호화
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