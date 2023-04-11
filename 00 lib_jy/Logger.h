#include <Windows.h>
#include <thread>
#include "LFObjectPool.h"

#define LOG(fileName, logLevel, format, ...) do { Logger::inst.Log(fileName, logLevel, format, __VA_ARGS__); }while(false)

#define LOG_LEVEL_FATAL	Logger::LogLevel::LEVEL_FATAL
#define LOG_LEVEL_ERROR Logger::LogLevel::LEVEL_ERROR
#define LOG_LEVEL_WARN  Logger::LogLevel::LEVEL_WARN	
#define LOG_LEVEL_INFO  Logger::LogLevel::LEVEL_INFO	
#define LOG_LEVEL_DEBUG Logger::LogLevel::LEVEL_DEBUG

#define LOG_SIZE 1024

struct Logger {
private:
	Logger();
public:
	~Logger();
	static Logger inst;

public:
	static enum class LogLevel :BYTE {
		LEVEL_FATAL = 0,
		LEVEL_ERROR = 1,
		LEVEL_WARN  = 2,
		LEVEL_INFO  = 3,
		LEVEL_DEBUG = 4, // Default
	};

	static struct LogData {
	public:
		LogData();
		~LogData();

	public:
		const char* fileName; // ("%s_%s.txt", __DATE__, fileName)
		Logger::LogLevel logLevel;
		char logStr[LOG_SIZE];
	};

private:
	bool shutDown = false;
	LogLevel logLevel = LOG_LEVEL_INFO;
	LFObjectPool<LogData> logData_pool;

private:
	// APC 처리 스레드 (JOB Thread)
	std::thread log_thread;
	void LogWorker(); 

	// APC (JOB func)
	static VOID NTAPI LogAPC(ULONG_PTR p_LoggingFunctor);

	// APC 처리 스레드 종료
	void Release();
	static VOID NTAPI ShutDownAPC(ULONG_PTR);

public:
	void SetLogLevel(const Logger::LogLevel& logLevel);
	void Log(const char* fileName, LogLevel logLevel, const char* format, ...);
};