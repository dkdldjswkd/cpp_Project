#include "Logger.h"
#include <iostream>
#include <Windows.h>

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
	if (inst.log_thread.joinable()) {
		inst.log_thread.join();
	}
}

void Logger::Set_LogLevel(const Logger::LogLevel& logLevel) {
	inst.logLevel = logLevel;
}

void Logger::Log_Worker() {
	for (;;)
		SleepEx(INFINITE, TRUE);
}

VOID Logger::Log_APC(ULONG_PTR p_logData){
	char file_name[512];
#pragma warning(suppress : 4996)
	sprintf(file_name, "%s_%s.txt", __DATE__, ((Logger::LogData*)p_logData)->type);

	FILE* fp;
	fopen_s(&fp, file_name, "at");
	if (fp != NULL) {
		fwrite(((Logger::LogData*)p_logData)->log_str, 1, strlen(((Logger::LogData*)p_logData)->log_str), fp);
		fclose(fp);
	}

	LOG_INST.logData_pool.Free((Logger::LogData*)p_logData);
}

void Logger::Log(const char* type, LogLevel logLevel, const char* format, ...) {
	if (logLevel > this->logLevel) return;

	LogData* p_logData = logData_pool.Alloc();
	p_logData->type = type;
	int len;

	switch (logLevel)
	{
		case Logger::LEVEL_FATAL:
#pragma warning(suppress : 4996)
			sprintf(p_logData->log_str, "[%s] [%s %s / %s] ", type, __DATE__, __TIME__, "FATAL");
			len = strlen(p_logData->log_str);
			break;

		case Logger::LEVEL_ERROR:
#pragma warning(suppress : 4996)
			sprintf(p_logData->log_str, "[%s] [%s %s / %s] ", type, __DATE__, __TIME__, "ERROR");
			len = strlen(p_logData->log_str);
			break;

		case Logger::LEVEL_WARN:
#pragma warning(suppress : 4996)
			sprintf(p_logData->log_str, "[%s] [%s %s / %s] ", type, __DATE__, __TIME__, "WARN");
			len = strlen(p_logData->log_str);
			break;

		case Logger::LEVEL_INFO:
#pragma warning(suppress : 4996)
			sprintf(p_logData->log_str, "[%s] [%s %s / %s] ", type, __DATE__, __TIME__, "INFO");
			len = strlen(p_logData->log_str);
			break;

		case Logger::LEVEL_DEBUG:
#pragma warning(suppress : 4996)
			sprintf(p_logData->log_str, "[%s] [%s %s / %s] ", type, __DATE__, __TIME__, "DEBUG");
			len = strlen(p_logData->log_str);
			break;
	}

	va_list var_list;
	va_start(var_list, format);
	vsnprintf(p_logData->log_str + len, LOG_SIZE - len - 1, format, var_list);
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