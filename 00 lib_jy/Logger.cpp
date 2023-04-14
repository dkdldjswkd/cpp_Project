#include "Logger.h"
#include <iostream>
#include <Windows.h>
using namespace std;

#define LOG_INST Logger::inst
Logger Logger::inst;

//------------------------------
// Logger
//------------------------------
Logger::Logger() {
	logLevel = LOG_LEVEL_DEBUG;
	logThread = thread([this]() { AlertableFunc(); });
}

Logger::~Logger() {
	shutDown = true;
	QueueUserAPC([](ULONG_PTR) { return; }, logThread.native_handle(), NULL);
	if (logThread.joinable()) logThread.join();
}

void Logger::AlertableFunc() {
	while (!shutDown) {
		SleepEx(INFINITE, TRUE);
	}
}

void Logger::SetLogLevel(const LogLevel& logLevel) {
	inst.logLevel = logLevel;
}

// 로그 스레드에 작업 큐잉
void Logger::Log(const char* fileName, const LogLevel& logLevel, const char* format, ...) {
	if (logLevel > this->logLevel) return;

	LogData* p_logData = logDataPool.Alloc();
	// Set fileName, logLevel
	strncpy_s(p_logData->fileName, FILE_NAME_SIZE, fileName, FILE_NAME_SIZE - 1);
	p_logData->logLevel = logLevel;
	// Set logStr
	va_list var_list;
	va_start(var_list, format);
	vsnprintf(p_logData->logStr, LOG_SIZE - 1, format, var_list);
	va_end(var_list);

	QueueUserAPC(Logger::LogAPC, logThread.native_handle(), (ULONG_PTR)p_logData);
}

// 파일 로깅 함수 (실질적, File I/O)
void Logger::LogAPC(ULONG_PTR p_logData) {
	FILE* fp;
	const auto& logData = *((Logger::LogData*)p_logData);

	// fopen
	char fileName[FILE_NAME_SIZE + 50];
#pragma warning(suppress : 4996)
	sprintf(fileName, "%s_%s.txt", __DATE__, logData.fileName);
	fopen_s(&fp, fileName, "at");

	// Logging
	char logBuf[LOG_SIZE];
	switch (logData.logLevel) {
		case LOG_LEVEL_FATAL:
			#pragma warning(suppress : 4996)
			snprintf(logBuf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.fileName, __DATE__, __TIME__, "FATAL");
			break;

		case LOG_LEVEL_ERROR:
			#pragma warning(suppress : 4996)
			snprintf(logBuf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.fileName, __DATE__, __TIME__, "ERROR");
			break;

		case LOG_LEVEL_WARN:
			#pragma warning(suppress : 4996)
			snprintf(logBuf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.fileName, __DATE__, __TIME__, "WARN");
			break;

		case LOG_LEVEL_INFO:
			#pragma warning(suppress : 4996)
			snprintf(logBuf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.fileName, __DATE__, __TIME__, "INFO");
			break;

		default:
			#pragma warning(suppress : 4996)
			snprintf(logBuf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.fileName, __DATE__, __TIME__, "DEBUG");
			break;
	}

	int logOffset = strlen(logBuf);
	#pragma warning(suppress : 4996)
	snprintf(logBuf + logOffset, (LOG_SIZE - 1) - logOffset, "%s\n", logData.logStr);
	logBuf[LOG_SIZE - 1] = 0;

	if (fp != NULL) {
		fprintf(fp, "%s", logBuf);
		fclose(fp);
	}

	inst.logDataPool.Free((Logger::LogData*)p_logData);
}