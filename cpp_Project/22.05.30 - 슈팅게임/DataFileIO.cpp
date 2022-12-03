#include "stdafx.h"
#include "DataFileIO.h"

char* p_tmp;

FILE* fp_reader;

bool Fread_to_ScreenBuf(const char* FileName) {
	fopen_s(&fp_reader, FileName, "rt");
	if (fp_reader == NULL) { printf("fp_reader == NULL, %s \n", FileName); return false; } // 수정하기, 디버깅 코드

	fread((void*)szScreenBuffer, dfSCREEN_HEIGHT * dfSCREEN_WIDTH, 1, fp_reader);
	fclose(fp_reader);

	return true;
}

bool Fread_to_buf(char* buf, size_t buf_size, const char* FileName) {
	fopen_s(&fp_reader, FileName, "rt");
	if (fp_reader == NULL) { printf("fp_reader == NULL, %s \n", FileName); return false; } // 수정하기, 디버깅 코드

	fread((void*)buf, buf_size, 1, fp_reader);
	fclose(fp_reader);

	return true;
}