#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#pragma warning(disable : 4996)
using namespace std;



int main() {
	// �۵� : src�� �ּҺ��� count byte ũ�� ��ŭ memcpy (null�� �����ٸ�, null�� ���������� cpy �� ��ȯ) 
	// ���� 1. dst �����÷ο츦 ����Ͽ� count�� �����ؾ���
	//			src�� null �͹̳����� ��Ʈ���� �ƴ϶��, src �����÷ο� �� �� ����.
	//			(src�� null �͹̳����� ��Ʈ���� �ֵ��� ����.)
	// ���� 2. ���� �� src ������ 0('\0')�� �����ٸ� *'0���� ����' �� ��ȯ.
	// ���� 3. * dst�� null �͹̳����Ͱ� �ƴ� �� ����

	char buf[5] = "1234";
	char buf2[100] = "5";
	strncpy(buf, buf2, 3);
	printf("%s \n", buf);
}