#include "MemoryScan.h"

using std::cout;
using std::hex;
using std::dec;
using std::endl;

void* IScanner::FindPattern(const char* module, const char* strPattern) {
	int patternLen = 0;
	int j = 0;

	unsigned char* skipByteMask = new unsigned char[MAX_PATTERN_SIZE];
	for (int k = 0; k < MAX_PATTERN_SIZE; k++)
		skipByteMask[k] = 0;
	unsigned char* pattern = Parse(patternLen, strPattern, skipByteMask);
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	MEMORY_BASIC_INFORMATION info;
	MODULEENTRY32 moduleEntry;
	GetModuleInfo(module, &moduleEntry);
	void* pStart = moduleEntry.modBaseAddr;
	void* pFinish = moduleEntry.modBaseAddr + moduleEntry.modBaseSize;
	unsigned char* current = (unsigned char*)pStart;
	for (; current < pFinish && j < patternLen; current++) {
		if (!VirtualQuery((LPCVOID)current, &info, sizeof(info)))
			continue;

		unsigned long long protectMask = PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE | PAGE_EXECUTE_READ;
		if (info.State == MEM_COMMIT && info.Protect & protectMask && !(info.Protect & PAGE_GUARD)) {
			unsigned long long finish = (unsigned long long)pFinish < (unsigned long long)info.BaseAddress + info.RegionSize ?
				(unsigned long long)pFinish : (unsigned long long)info.BaseAddress + info.RegionSize;
			current = (unsigned char*)info.BaseAddress;
			unsigned char* rip = 0;
			for (unsigned long long k = (unsigned long long)info.BaseAddress; k < finish && j < patternLen; k++, current++) {
				if (skipByteMask[j] || pattern[j] == *current) {
					if (j == 0)
						rip = current;
					j++;
				}
				else {
					j = 0;
					if (pattern[0] == *current) {
						rip = current;
						j = 1;
					}
				}

			}
			if (j == patternLen) {
				current = rip;
				break;
			}
		}
		else
			current += sysInfo.dwPageSize;
	}
	delete[] skipByteMask;
	delete[] pattern;
	if (j == patternLen)
		return (void*)current;
	return nullptr;
}

int IScanner::GetModuleInfo(const char* name, MODULEENTRY32* entry) {
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, GetCurrentProcessId());
	if (snapshot == INVALID_HANDLE_VALUE)
		return 1;
	entry->dwSize = sizeof(MODULEENTRY32);
	if (!Module32First(snapshot, entry)) {
		CloseHandle(snapshot);
		return 1;
	}
	do {
		if (!_stricmp(entry->szModule, name))
			break;
	} while (Module32Next(snapshot, entry));
	CloseHandle(snapshot);
	return 0;
}

unsigned char* IScanner::Parse(int& len, const char* strPattern, unsigned char* skipByteMask) {
	int strPatternLen = strlen(strPattern);
	unsigned char* pattern = new unsigned char[strPatternLen];
	for (int i = 0; i < strPatternLen; i++)
		pattern[i] = 0;
	len = 0;
	for (int i = 0; i < strPatternLen; i += 2) {
		unsigned char code = 0;
		if (strPattern[i] == SKIP_SYMBOL)
			skipByteMask[len] = 1;
		else 
			code = Parse(strPattern[i]) * 16 + Parse(strPattern[i + 1]);
		i++;
		pattern[len++] = code;
	}
	return pattern;
}

unsigned char IScanner::Parse(char byte) { // some magic values
	if (byte >= '0' && byte <= '9')
		return byte - '0';
	else if (byte >= 'a' && byte <= 'f')
		return byte - 'a' + 10;
	else if (byte >= 'A' && byte <= 'F')
		return byte - 'A' + 10;
	return 0;
}

void ConsoleScanner::PrintMemory(const char* title, unsigned char* memPointer, int size) {
	cout << title << ":" << endl;
	for (int i = 0; i < size; i++)
		cout << hex << (int)(*(memPointer + i)) << " ";
	cout << endl;
	cout << dec;
}

void FileScanner::PrintMemory(const char* title, unsigned char* memPointer, int size) {
	fprintf(fptr, "%s:\n", title);
	for (int i = 0; i < size; i++)
		fprintf(fptr, "%x ", (int)(*(memPointer + i)));
	fprintf(fptr, "\n", title);
}
