#pragma once
#include <Windows.h>
#include <timeapi.h>

//#define UNUSE_PROFILE

#ifdef UNUSE_PROFILE
#define PRO_BEGIN(TagName)
#define PRO_END(TagName)
#define PRO_RESET()		
#define PRO_DATAOUT()	
#endif

#ifndef UNUSE_PROFILE
#define PRO_BEGIN(TagName)	Profiler::inst.ProfileBegin(TagName)
#define PRO_END(TagName)	Profiler::inst.ProfileEnd(TagName)
#define PRO_RESET()			Profiler::inst.ProfileReset()
#define PRO_FILEOUT()		Profiler::inst.ProfileFileOut()
#endif

#define MAX_THREAD_NUM		20
#define PROFILE_DATA_NUM	100
#define PROFILE_NAME_LEN	128

class Profiler {
private:
	Profiler();
public:
	~Profiler();
	static Profiler inst;

private:
	static struct ProfileData {
	public:
		bool useFlag = false;
		bool resetFlag = false;
		char name[PROFILE_NAME_LEN] = {0, }; // 프로파일 샘플 이름.

	public:
		DWORD64 totalTime	= 0;
		DWORD	totalCall	= 0;
		DWORD	startTime	= 0; // ProfileBegin

		// 평균 계산 시 제외하기 위함
		DWORD	max[2] = { 0, 0 };				
		DWORD	min[2] = { MAXDWORD, MAXDWORD };

	public:
		// 제외시킬 데이터라면 ret true (max, min 동시비교 안함)
		bool VaildateData(DWORD profileTime) {
			for (int i = 0; i < 2; i++) {
				if (max[i] < profileTime) {
					max[i] = profileTime;
					return true;
				}
			}
			for (int i = 0; i < 2; i++) {
				if (min[i] > profileTime) {
					min[i] = profileTime;
					return true;
				}
			}
			return false;
		}
		void Init() {
			totalTime	= 0;
			totalCall	= 0;
			max[0] = 0;
			max[1] = 0;
			min[0] = MAXDWORD;
			min[1] = MAXDWORD;
		}
	};

private:
	const DWORD tlsIndex;
	int profileIndex = -1;
	ProfileData profileData[MAX_THREAD_NUM][PROFILE_DATA_NUM];

private:
	ProfileData* Get();

public:
	void ProfileBegin(const char* name);
	void ProfileEnd(const char* name);
	void ProfileFileOut();
	void ProfileReset();
};