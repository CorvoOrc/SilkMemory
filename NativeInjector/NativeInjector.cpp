#include <conio.h>
#include <functional>
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

using namespace std;

void GetDllPath(const char* in_dllName, string& out_dllPath);
void Inject(string& dllPath, string& processName);
DWORD GetProcessIdentificator(string& processName);

const int PATH_LEN = 128;

int main() {
	cout << "Entry point" << endl;
	string dllPath;
	GetDllPath("dx9Dll.dll", dllPath);
	string processName = "Dota2.exe";
	//string processName = "StateManager.exe";
	Inject(dllPath, processName);
}

void GetDllPath(const char* in_dllName, string& out_dllPath) {
	char currentPath[PATH_LEN];
	GetModuleFileName(NULL, currentPath, PATH_LEN);
	out_dllPath = currentPath;
	out_dllPath = out_dllPath.substr(0, out_dllPath.find_last_of('\\') + 1);
	out_dllPath.append(in_dllName);
}

void Inject(string& dllPath, string& processName) {
	DWORD processId = GetProcessIdentificator(processName);
	if (processId == NULL)
		throw invalid_argument("Process dont existed");

	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | 
		PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, processId);
	HMODULE hModule = GetModuleHandle("kernel32.dll");
    FARPROC address = GetProcAddress(hModule, "LoadLibraryA");

	int payloadSize = sizeof(char) * dllPath.length() + 1;
	LPVOID allocAddress = VirtualAllocEx(hProcess, NULL, payloadSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	SIZE_T written;
	bool writeResult = WriteProcessMemory(hProcess, allocAddress, dllPath.c_str(), payloadSize, &written);
	DWORD treadId;
	CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)address, allocAddress, 0, &treadId);
	CloseHandle(hProcess);
}

DWORD GetProcessIdentificator(string& processName) {
	PROCESSENTRY32 processEntry;
	processEntry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	DWORD processId = NULL;
	if (Process32First(snapshot, &processEntry)) {
		while (Process32Next(snapshot, &processEntry)) {
			if (!_stricmp(processEntry.szExeFile, processName.c_str())) {
				processId = processEntry.th32ProcessID;
				break;
			}
		}
	}
	CloseHandle(snapshot);
	return processId;
}