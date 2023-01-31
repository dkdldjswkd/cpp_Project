#include "Logger.h"
#include <iostream>
#include <Windows.h>
using namespace std;

// 오후 1:49 2023-01-31
// LFObjectPool.h 필요

#define LOG_INST Logger::inst

//------------------------------
// Logger
//------------------------------
Logger::Logger() {
	inst.logLevel = LOG_LEVEL_DEBUG;
	inst.log_thread = thread([]() {inst.Log_Worker(); });
}
Logger::~Logger() {
	inst.Release();
	if (inst.log_thread.joinable()) {
		log_thread.join();
	}
}
Logger Logger::inst;

void Logger::Release(){
	shutDown = true;
	QueueUserAPC(Logger::ShutDown_APC, log_thread.native_handle(), NULL);

	if (inst.log_thread.joinable()) {
		inst.log_thread.join();
	}
}

void Logger::Set_LogLevel(const Logger::LogLevel& logLevel) {
	inst.logLevel = logLevel;
}

void Logger::Log_Worker() {
	while (!shutDown) {
		SleepEx(INFINITE, TRUE);
	}
}

// * fileName은 리터럴이어야함, format에 대한 문자열 복사
void Logger::Log(const char* fileName, LogLevel logLevel, const char* format, ...) {
	if (logLevel > this->logLevel) return;

	LogData* p_logData = logData_pool.Alloc();
	p_logData->fileName = fileName;
	p_logData->logLevel = logLevel;

	va_list var_list;
	va_start(var_list, format);
	vsnprintf(p_logData->log_str, LOG_SIZE - 1, format, var_list);
	va_end(var_list);

	QueueUserAPC(Logger::Log_APC, log_thread.native_handle(), (ULONG_PTR)p_logData);
}

// * 실질 코드 진행 부, 
VOID Logger::Log_APC(ULONG_PTR p_logData) {
	FILE* fp;
	const auto& logData = *((Logger::LogData*)p_logData);

	// fopen
	char file_name[128];
#pragma warning(suppress : 4996)
	sprintf(file_name, "%s_%s.txt", __DATE__, logData.fileName);
	fopen_s(&fp, file_name, "at");

	// Logging
	char log_buf[LOG_SIZE];
	switch (logData.logLevel) {
		case LOG_LEVEL_FATAL:
			#pragma warning(suppress : 4996)
			snprintf(log_buf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.fileName, __DATE__, __TIME__, "FATAL");
			break;

		case LOG_LEVEL_ERROR:
			#pragma warning(suppress : 4996)
			snprintf(log_buf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.fileName, __DATE__, __TIME__, "ERROR");
			break;

		case LOG_LEVEL_WARN:
			#pragma warning(suppress : 4996)
			snprintf(log_buf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.fileName, __DATE__, __TIME__, "WARN");
			break;

		case LOG_LEVEL_INFO:
			#pragma warning(suppress : 4996)
			snprintf(log_buf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.fileName, __DATE__, __TIME__, "INFO");
			break;

		case LOG_LEVEL_DEBUG:
			#pragma warning(suppress : 4996)
			snprintf(log_buf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.fileName, __DATE__, __TIME__, "DEBUG");
			break;
	}

	int log_offset = strlen(log_buf);
	#pragma warning(suppress : 4996)
	snprintf(log_buf + log_offset, LOG_SIZE - (log_offset + 1), "%s\n", logData.log_str);
	log_buf[LOG_SIZE - 1] = 0;

	if (fp != NULL) {
		fprintf(fp, "%s", log_buf);
		fclose(fp);
	}

	LOG_INST.logData_pool.Free((Logger::LogData*)p_logData);
}

VOID Logger::ShutDown_APC(ULONG_PTR){
	return;
}



//------------------------------
// Logger::LogData
//------------------------------
Logger::LogData::LogData() {

}

Logger::LogData::~LogData() {

}