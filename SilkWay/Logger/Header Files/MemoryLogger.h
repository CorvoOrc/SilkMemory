#pragma once
#include <cstdio>
#include <cstdarg>

const int MAX_LOG_SIZE = 256;

class ILogger {
protected:
	ILogger(const char* _path) {
		path = path;
	}
public:
	virtual ~ILogger() { }
	virtual void Log(const char* format, ...) = 0;
protected:
	const char* path;
};

class MemoryLogger : public ILogger {
public:
	MemoryLogger(const char* _path) : ILogger(_path) {
		fopen_s(&fptr, _path, "w+");
	}
	~MemoryLogger() {
		fclose(fptr);
	}
	void Log(const char* format, ...) {
		char log[MAX_LOG_SIZE]; log[MAX_LOG_SIZE - 1] = 0;
		va_list args;
		va_start(args, format);
		vsprintf_s(log, MAX_LOG_SIZE, format, args);
		va_end(args);
		fprintf(fptr, log);
	}
protected:
	FILE* fptr;
};