#include <Windows.h>
#include <thread>
#include "LFObjectPool.h"

// 오전 1:48 2023-01-09
// 원본

#define LOG_SIZE 1024
#define LOG_INST				Logger::inst

#define LOG_INIT()				do { Logger::inst.Init();	 } while (false);
#define LOG_RELEASE()			do { Logger::inst.Release(); } while (false);
#define LOG(type, format, ...)	do { Logger::inst.Log(type, format, __VA_ARGS__); }while(false);

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
		LEVEL_WARN = 2,
		LEVEL_INFO = 3, // Default
		LEVEL_DEBUG = 4,
	};
#define LOG_LEVEL_FATAL	Logger::LogLevel::LEVEL_FATAL
#define LOG_LEVEL_ERROR Logger::LogLevel::LEVEL_ERROR
#define LOG_LEVEL_WARN  Logger::LogLevel::LEVEL_WARN	
#define LOG_LEVEL_INFO  Logger::LogLevel::LEVEL_INFO	
#define LOG_LEVEL_DEBUG Logger::LogLevel::LEVEL_DEBUG

public:
	static struct LogData {
	public:
		LogData();
		~LogData();

	public:
		const char* type;
		Logger::LogLevel logLevel;
		char log_str[LOG_SIZE];
	};

private:
	bool shutDown = false;
	bool initialised = false;
	std::thread log_thread;
	LogLevel logLevel = LOG_LEVEL_INFO;
	J_LIB::LFObjectPool<LogData> logData_pool;

private:
	void Log_Worker();
	static VOID NTAPI Log_APC(ULONG_PTR p_LoggingFunctor);
	static VOID NTAPI ShutDown_APC(ULONG_PTR);

public:
	bool Init();
	void Release();
	void Set_LogLevel(const Logger::LogLevel& logLevel);
	void Log(const char* type, LogLevel logLevel, const char* format, ...);
};