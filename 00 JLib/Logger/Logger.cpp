#include "Logger.h"
#include <iostream>
#include <Windows.h>

// 오전 1:48 2023-01-09
// 원본

using namespace std;

//------------------------------
// Logger
//------------------------------
Logger::Logger() {}
Logger::~Logger() {}
Logger Logger::inst;

bool Logger::Init() {
	if (inst.initialised)
		return false;

	inst.logLevel = LOG_LEVEL_INFO;
	inst.initialised = true;
	inst.log_thread = thread([]() {inst.Log_Worker();});
	return true;
}

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

VOID Logger::Log_APC(ULONG_PTR p_logData){
	FILE* fp;
	const auto& logData = *((Logger::LogData*)p_logData);

	// fopen
	char file_name[128];
#pragma warning(suppress : 4996)
	sprintf(file_name, "%s_%s.txt", __DATE__, logData.type);
	fopen_s(&fp, file_name, "at");

	// Logging
	char log_buf[LOG_SIZE];
	log_buf[0] = 0;

	switch (logData.logLevel)
	{
		case LOG_LEVEL_FATAL:
			#pragma warning(suppress : 4996)
			snprintf(log_buf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.type, __DATE__, __TIME__, "FATAL");
			break;

		case LOG_LEVEL_ERROR:
			#pragma warning(suppress : 4996)
			snprintf(log_buf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.type, __DATE__, __TIME__, "ERROR");
			break;

		case LOG_LEVEL_WARN:
			#pragma warning(suppress : 4996)
			snprintf(log_buf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.type, __DATE__, __TIME__, "WARN");
			break;

		case LOG_LEVEL_INFO:
			#pragma warning(suppress : 4996)
			snprintf(log_buf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.type, __DATE__, __TIME__, "INFO");
			break;

		case LOG_LEVEL_DEBUG:
			#pragma warning(suppress : 4996)
			snprintf(log_buf, LOG_SIZE - 1, "[%s] [%s %s / %s] ", logData.type, __DATE__, __TIME__, "DEBUG");
			break;
	}

	int log_offset = strlen(log_buf);
	#pragma warning(suppress : 4996)
	snprintf(log_buf + log_offset, LOG_SIZE - log_offset - 1, "%s\n", logData.log_str);

	if (fp != NULL) {
		fwrite(((Logger::LogData*)p_logData)->log_str, 1, strlen(((Logger::LogData*)p_logData)->log_str), fp);
		fclose(fp);
	}

	LOG_INST.logData_pool.Free((Logger::LogData*)p_logData);
}

VOID Logger::ShutDown_APC(ULONG_PTR){
	return;
}

void Logger::Log(const char* type, LogLevel logLevel, const char* format, ...) {
	if (logLevel > this->logLevel) return;

	LogData* p_logData = logData_pool.Alloc();
	p_logData->type = type;
	p_logData->logLevel = logLevel;

	va_list var_list;
	va_start(var_list, format);
	vsnprintf(p_logData->log_str, LOG_SIZE - 1, format, var_list);
	va_end(var_list);

	QueueUserAPC(Logger::Log_APC, log_thread.native_handle(), (ULONG_PTR)p_logData);
}

//------------------------------
// Logger::LogData
//------------------------------
Logger::LogData::LogData() {

}

Logger::LogData::~LogData() {

}