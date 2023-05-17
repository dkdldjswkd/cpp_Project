#include "Profiler.h"
#include <string>
#pragma comment(lib, "Winmm.lib")

//------------------------------
// Profiler
//------------------------------

Profiler::Profiler() :tlsIndex(TlsAlloc()) {
}

Profiler::~Profiler() {
}

Profiler::ProfileData* Profiler::Get() {
	ProfileData* profileArr = (ProfileData*)TlsGetValue(tlsIndex);
	if (profileArr == nullptr) {
		profileArr = profileData[InterlockedIncrement((LONG*)&profileIndex)];
		TlsSetValue(tlsIndex, (LPVOID)profileArr);
	}
	return profileArr;
}

void Profiler::ProfileEnd(const char* name) {
	ProfileData* profileArr = Get();

	for (int i = 0; i < PROFILE_DATA_NUM; i++) {
		// ProfileData 검색
		if (false == profileArr[i].useFlag) return;
		if (strcmp(profileArr[i].name, name) != 0)
			continue;

		// Profile Reset, 데이터 반영 x
		if (true == profileArr[i].resetFlag) {
			profileArr[i].Init();
			profileArr[i].resetFlag = false;
			return;
		}

		// profile time이 최대/최소 값이라면, 반영 x
		DWORD profileTime = timeGetTime() - profileArr[i].startTime;
		if (profileArr[i].VaildateData(profileTime)) return;

		// profile time 반영 o
		profileArr[i].totalTime += profileTime;
		profileArr[i].totalCall++;
	}
}

void Profiler::ProfileBegin(const char* name) {
	ProfileData* profileArr = Get();

	// 프로파일 된적 있음
	for (int i = 0; i < PROFILE_DATA_NUM; i++) {
		if (strcmp(profileArr[i].name, name) == 0) {
			profileArr[i].startTime = timeGetTime();
			return;
		}
	}

	// 프로파일 처음
	for (int i = 0; i < PROFILE_DATA_NUM; i++) {
		if (false == profileArr[i].useFlag) {
			strncpy_s(profileArr[i].name, PROFILE_NAME_LEN, name, PROFILE_NAME_LEN - 1);
			profileArr[i].useFlag = true;
			profileArr[i].startTime = timeGetTime();
			return;
		}
	}
}

void Profiler::ProfileFileOut() {
	// fopen
	FILE* fp;
	char fileName[128];
	#pragma warning(suppress : 4996)
	sprintf(fileName, "%s_%s.csv", "Proflie", __DATE__);
	fopen_s(&fp, fileName, "wt");
	if (fp == NULL) return;

	// file I/O
	for (int i = 0; i <= profileIndex; i++) {
		ProfileData* profileArr = profileData[i];
		for (int j = 0; j < PROFILE_DATA_NUM; j++) {
			if (false == profileArr[j].useFlag)	break;

			fprintf(fp, "%s \n", profileArr[j].name);
			fprintf(fp, "average, total time, call num, min, max\n");
			fprintf(fp, "%f ms, %llu ms, %u, %u, %u \n\n", (float)(profileArr[j].totalTime / (float)profileArr[j].totalCall), profileArr[j].totalTime, profileArr[j].totalCall, profileArr[j].min[0], profileArr[j].max[0]);
		}
	}

	fclose(fp);
}

void Profiler::ProfileReset() {
	for (int i = 0; i <= profileIndex; i++) {
		for (int j = 0; j < PROFILE_DATA_NUM; j++) {
			if (false == profileData[i][j].useFlag) break;
			profileData[i][j].resetFlag = true;
		}
	}
}