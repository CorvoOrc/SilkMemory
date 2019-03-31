#pragma once
#include <iostream>
#include "Windows.h"
#include "tlhelp32.h"

const char SKIP_SYMBOL = '??';
const int MAX_PATTERN_SIZE = 100;

class IScanner {
protected:
	IScanner() { }
public:
	virtual ~IScanner() { }
	virtual void PrintMemory(const char* title, unsigned char* memPointer, int size) = 0;
	virtual void* FindPattern(const char* module, const char* strPattern);
	int GetModuleInfo(const char* name, MODULEENTRY32* entry);
private:
	unsigned char* Parse(int& len, const char* strPattern, unsigned char* skipByteMask);
	unsigned char Parse(char byte);
};

class ConsoleScanner : public IScanner {
public:
	ConsoleScanner() : IScanner() { }
	~ConsoleScanner() { }
	void PrintMemory(const char* title, unsigned char* memPointer, int size);
};

class FileScanner : public IScanner {
public:
	FileScanner(const char* _path) : IScanner() { 
		fopen_s(&fptr, _path, "w+");
	}
	~FileScanner() { 
		fclose(fptr);
	}
	void PrintMemory(const char* title, unsigned char* memPointer, int size);
protected:
	FILE* fptr;
};

