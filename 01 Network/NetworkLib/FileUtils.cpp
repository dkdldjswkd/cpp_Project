#include <iostream>
#include "FileUtils.h"

void LoadFile(const char* fileName, char** dst) {
	FILE* fp = NULL;
	fopen_s(&fp, fileName, "rt");

	// Get fileSize
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);
	rewind(fp);

	*dst = (char*)malloc(fileSize + 1);
	fread(*dst, 1, fileSize, fp);
	(*dst)[fileSize] = '\0';
	fclose(fp);
}

//void LoadFileW(const wchar_t* fileName, wchar_t** dst) {
//	FILE* fp = NULL;
//	_wfopen_s(&fp, fileName, L"rt");
//
//	fseek(fp, 0, SEEK_END);
//	int fileSize = ftell(fp);
//	rewind(fp);
//
//	// Allocate memory for the buffer
//	*dst = (wchar_t*)malloc(sizeof(wchar_t) * (fileSize + 1));
//	if (*dst == NULL) {
//		fclose(fp);
//	}
//
//	// Read the file into the buffer
//	fread(*dst, sizeof(wchar_t), fileSize, fp);
//	(*dst)[fileSize / sizeof(wchar_t)] = L'\0';
//
//	fclose(fp);
//}