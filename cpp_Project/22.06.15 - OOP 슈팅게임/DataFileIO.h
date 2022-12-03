#pragma once
#include "ScreenBuffer.h"

extern char tmpBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];
extern char* p_tmp;

bool Fread_to_ScreenBuf(const char* FileName);
bool Fread_to_buf(char* buf, size_t buf_size, const char* FileName);