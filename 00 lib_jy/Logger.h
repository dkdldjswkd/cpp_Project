#include <Windows.h>
#include <thread>	
#include "LFObjectPool.h"

#define LOG(fileName, logLevel, format, ...) do { Logger::GetInst().Log(fileName, logLevel, format, __VA_ARGS__); }while(false)

#define LOG_LEVEL_FATAL LogLevel::LEVEL_FATAL
#define LOG_LEVEL_ERROR LogLevel::LEVEL_ERROR
#define LOG_LEVEL_WARN  LogLevel::LEVEL_WARN	
#define LOG_LEVEL_INFO  LogLevel::LEVEL_INFO	
#define LOG_LEVEL_DEBUG LogLevel::LEVEL_DEBUG

#define FILE_NAME_SIZE	128
#define LOG_SIZE		1024

enum class LogLevel : BYTE {
	LEVEL_FATAL = 0,
	LEVEL_ERROR = 1,
	LEVEL_WARN  = 2,
	LEVEL_INFO  = 3,
	LEVEL_DEBUG = 4, // Default
};

class Logger {
private:
	Logger();
public:
	~Logger();

private:
	struct LogData {
	public:
		LogData() {}
		~LogData() {}

	public:
		char fileName[FILE_NAME_SIZE];
		char logStr[LOG_SIZE];
		LogLevel logLevel;
	};

private:
	bool shutDown = false;
	LogLevel logLevel = LOG_LEVEL_INFO;
	LFObjectPool<LogData> logDataPool;
	std::thread logThread;

private:
	void AlertableFunc(); // logThread call, LogAPC 처리 
	static void LogAPC(ULONG_PTR p_logData); // logThread에서 실질적으로 처리됨

public:
	static Logger& GetInst();
	void SetLogLevel(const LogLevel& logLevel);
	void Log(const char* fileName, const LogLevel& logLevel, const char* format, ...);
};