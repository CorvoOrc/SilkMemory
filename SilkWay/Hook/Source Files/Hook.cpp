#include "Hook.h"

namespace silk_way {
	int IHook::Freeze() {
		THREADENTRY32 te32;
		HANDLE hThread = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (hThread == INVALID_HANDLE_VALUE)
			return ERROR_ENUM_THREAD_START;

		te32.dwSize = sizeof(THREADENTRY32);

		if (!Thread32First(hThread, &te32)) {
			CloseHandle(hThread);
			return ERROR_ENUM_THREAD_START;
		}

		DWORD dwOwnerPID = GetCurrentProcessId();
		do {
			if (te32.th32OwnerProcessID == dwOwnerPID && te32.th32ThreadID != GetCurrentThreadId()) {
				HANDLE openThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
				if (openThread != NULL) {
					SuspendThread(openThread);
					CloseHandle(openThread);
				}
			}
		} while (Thread32Next(hThread, &te32));
		CloseHandle(hThread);
		return SUCCESS_CODE;
	}

	int IHook::Unfreeze() {
		THREADENTRY32 te32;
		HANDLE hThread = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (hThread == INVALID_HANDLE_VALUE)
			return ERROR_ENUM_THREAD_START;

		te32.dwSize = sizeof(THREADENTRY32);

		if (!Thread32First(hThread, &te32)) {
			CloseHandle(hThread);
			return ERROR_ENUM_THREAD_START;
		}

		DWORD dwOwnerPID = GetCurrentProcessId();
		do {
			if (te32.th32OwnerProcessID == dwOwnerPID && te32.th32ThreadID != GetCurrentThreadId()) {
				HANDLE openThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
				if (openThread != NULL) {
					ResumeThread(openThread);
					CloseHandle(openThread);
				}
			}
		} while (Thread32Next(hThread, &te32));
		CloseHandle(hThread);
		return SUCCESS_CODE;
	}
}