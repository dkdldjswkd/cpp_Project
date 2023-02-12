#include "Profiler.h"
#include <string>
#pragma comment(lib, "Winmm.lib")

//------------------------------
// Profiler
//------------------------------

Profiler Profiler::inst;

Profiler::Profiler() :tls_index(TlsAlloc()) {
}

Profiler::~Profiler() {
}

Profiler::ProfileData* Profiler::GetTLS_ProfileArray() {
	ProfileData* profile_array = (ProfileData*)TlsGetValue(tls_index);
	if (profile_array == nullptr) {
		auto thread_id = GetThreadId(GetCurrentThread());
		auto index = InterlockedIncrement((LONG*)&profile_index);
		profile_array = profileData[index];
		TlsSetValue(tls_index, (LPVOID)profile_array);
	}

	return profile_array;
}

void Profiler::ProfileBegin(const char* name) {
	ProfileData* profile_array = GetTLS_ProfileArray();

	// 프로파일 된적 있음
	for (int i = 0; i < PROFILE_DATA_NUM; i++) {
		if (strcmp(profile_array[i].name, name) == 0) {
			profile_array[i].start_time = timeGetTime();
			return;
		}
	}

	// 프로파일 처음
	for (int i = 0; i < PROFILE_DATA_NUM; i++) {
		if (false == profile_array[i].is_use) {
#pragma warning(suppress : 4996)
			strncpy(profile_array[i].name, name, PROFILE_NAME_LEN - 1);
			profile_array[i].is_use = true;
			profile_array[i].start_time = timeGetTime();
			return;
		}
	}
}

void Profiler::ProfileEnd(const char* name) {
	ProfileData* profile_array = GetTLS_ProfileArray();

	for (int i = 0; i < PROFILE_DATA_NUM; i++) {
		//------------------------------
		// ProfileData 검색
		//------------------------------
		if (false == profile_array[i].is_use)
			continue;
		if (strcmp(profile_array[i].name, name) != 0)
			continue;

		//------------------------------
		// ProfileData 반영 X (Reset or Invalid data)
		//------------------------------
		if (true == profile_array[i].reset_flag) {
			profile_array[i].Reset();
			profile_array[i].reset_flag = false;
			break;
		}
		DWORD delta_time = timeGetTime() - profile_array[i].start_time;
		if (profile_array[i].Check_InvalidData(delta_time))
			break;

		//------------------------------
		// ProfileData 반영 O
		//------------------------------
		profile_array[i].total_time += delta_time;
		profile_array[i].call_num++;
	}
}

void Profiler::ProfileFileOut() {
	// fopen
	FILE* fp;
	char file_name[128];
#pragma warning(suppress : 4996)
	sprintf(file_name, "%s_%s.csv", "Proflie", __DATE__);
	fopen_s(&fp, file_name, "wt");
	if (fp == NULL) 
		return;

	// file I/O
	for (int i = 0; i <= profile_index; i++) {
		ProfileData* profile_array = profileData[i];
		for (int j = 0; j < PROFILE_DATA_NUM; j++) {
			if (false == profile_array[j].is_use)
				break;

			fprintf(fp, "%s \n", profile_array[j].name);
			fprintf(fp, "average, total time, call num, min, max\n");
			fprintf(fp, "%f ms, %llu ms, %u, %u, %u \n\n",
				(float)(profile_array[j].total_time / (float)profile_array[j].call_num), profile_array[j].total_time, profile_array[j].call_num, profile_array[j].min_time[0], profile_array[j].max_time[0]);
		}
	}

	fclose(fp);
}

void Profiler::ProfileReset() {
	for (int i = 0; i <= profile_index; i++) {
		for (int j = 0; j < PROFILE_DATA_NUM; j++) {
			if (false == profileData[i][j].is_use)
				break;
			profileData[i][j].reset_flag = true;
		}
	}
}