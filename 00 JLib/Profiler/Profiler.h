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
		bool is_use = false;
		char name[PROFILE_NAME_LEN] = {0, }; // �������� ���� �̸�.
		bool reset_flag = false;

	public:
		DWORD64 total_time	= 0;
		DWORD	call_num	= 0;
		DWORD	start_time	= 0; // ProfileBegin
														// �Ʒ� ���� �����ϰ� ���ġ ���
		DWORD	max_time[2] = { 0, 0 };					// 0 > 1, 0�� �ε��� ���� �� ŭ
		DWORD	min_time[2] = { MAXDWORD, MAXDWORD };	// 0 < 1, 0 �� �ε��� ���� �� ����

	public:
		// ���ܽ�ų �����Ͷ�� ret true (max, min ���ú� ����)
		bool Check_InvalidData(DWORD profile_time) {
			for (int i = 0; i < 2; i++) {
				if (max_time[i] < profile_time) {
					max_time[i] = profile_time;
					return true;
				}
			}

			for (int i = 0; i < 2; i++) {
				if (min_time[i] > profile_time) {
					min_time[i] = profile_time;
					return true;
				}
			}

			return false;
		}

		void Reset() {
			total_time	= 0;
			call_num	= 0;
			max_time[0] = 0;
			max_time[1] = 0;
			min_time[0] = MAXDWORD;
			min_time[1] = MAXDWORD;
		}
	};

private:
	const DWORD tls_index;
	int profile_index = -1;
	ProfileData profileData[MAX_THREAD_NUM][PROFILE_DATA_NUM];

private:
	ProfileData* GetTLS_ProfileArray();

public:
	void ProfileBegin(const char* name);
	void ProfileEnd(const char* name);
	void ProfileFileOut();
	void ProfileReset();
};