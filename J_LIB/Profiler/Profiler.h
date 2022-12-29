#pragma once
#include <Windows.h>

#define PRO_BEGIN(TagName)	ProfileBegin(TagName)
#define PRO_END(TagName)	ProfileEnd(TagName)

#define PROFILE_DATA_NUM	100
#define MAX_THREAD_NUM		20

class Profiler {
private:
	Profiler();
public:
	~Profiler();
	static Profiler inst;

private:
	static struct ProfileData {
		bool is_use = false;
		const char name[64] = {0, }; // �������� ���� �̸�.

		LARGE_INTEGER start_time; // �������� ���� ���� �ð�.

		long long total_time; // ��ü ���ð� ī���� Time. (��½� ȣ��ȸ���� ������ ��� ����)
		long long min_time[2]; // �ּ� ���ð� ī���� Time. (�ʴ����� ����Ͽ� ���� / [0] �����ּ� [1])
		long long max_time[2]; // �ִ� ���ð� ī���� Time. (�ʴ����� ����Ͽ� ���� / [0] �����ִ� [1])
		long long call_num; // ���� ȣ�� Ƚ��.
	};

private:
	const int tls_index;
	int profile_index = -1;
	ProfileData profileData[MAX_THREAD_NUM][PROFILE_DATA_NUM];

public:
	// ...

private:
	// ...

public:
	void ProfileBegin(const char* name);
	void ProfileEnd(const char* name);
	void ProfileReset();
	void ProfileDataOutText(const WCHAR* fileName);
};