#include "Profiler.h"

//------------------------------
// Profiler
//------------------------------

Profiler::Profiler() :tls_index(TlsAlloc()) {
}

Profiler::~Profiler() {
}

void Profiler::ProfileBegin(const char* name){
	ProfileData* pro = (ProfileData*)TlsGetValue(tls_index);
	if (pro == nullptr) {
		auto index = InterlockedIncrement((LONG*)&profile_index);
		pro = profileData[index];
		TlsSetValue(index, (LPVOID)pro);
	}

	for (int i = 0; i < PROFILE_DATA_NUM; i++) {
		if (strcmp(pro[i].name, name) == 0) {
		}
	}
}