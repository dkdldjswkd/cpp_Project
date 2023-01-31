#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#pragma warning(disable : 4996)
using namespace std;



int main() {
	// 작동 : src의 주소부터 count byte 크기 만큼 memcpy (null을 만났다면, null을 마지막으로 cpy 후 반환) 
	// 주의 1. dst 오버플로우를 고려하여 count를 설정해야함
	//			src가 null 터미네이터 스트링이 아니라면, src 오버플로우 될 수 있음.
	//			(src는 null 터미네이터 스트링을 넣도록 하자.)
	// 주의 2. 복사 중 src 데이터 0('\0')을 만났다면 *'0까지 복사' 후 반환.
	// 주의 3. * dst가 null 터미네이터가 아닐 수 있음

	char buf[5] = "1234";
	char buf2[100] = "5";
	strncpy(buf, buf2, 3);
	printf("%s \n", buf);
}