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
		const char name[64] = {0, }; // 프로파일 샘플 이름.

		LARGE_INTEGER start_time; // 프로파일 샘플 실행 시간.

		long long total_time; // 전체 사용시간 카운터 Time. (출력시 호출회수로 나누어 평균 구함)
		long long min_time[2]; // 최소 사용시간 카운터 Time. (초단위로 계산하여 저장 / [0] 가장최소 [1])
		long long max_time[2]; // 최대 사용시간 카운터 Time. (초단위로 계산하여 저장 / [0] 가장최대 [1])
		long long call_num; // 누적 호출 횟수.
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