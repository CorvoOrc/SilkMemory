#include "HardwareBPHook.h"

namespace silk_way {
	void HardwareBPHook::SetExceptionHandler(PVECTORED_EXCEPTION_HANDLER pVecExcHandler) {
		pException = AddVectoredExceptionHandler(1, pVecExcHandler);
	}

	int HardwareBPHook::GetFreeReg(unsigned long long* mask) {
		int freeReg = -1;
		for (int i = 0; i < 4; i++) {
			if ((*mask & (1ULL << (i * 2))) == 0) {
				freeReg = i;
				break;
			}
		}
		return freeReg;
	}

	int HardwareBPHook::SetHook(void* source, void* destination) {
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
		bool isRegDefined = false;
		int freeReg = -1;
		Freeze();
		do {
			if (te32.th32OwnerProcessID == dwOwnerPID) {
				HANDLE openThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
				if (!isRegDefined) {
					CONTEXT context;
					ZeroMemory(&context, sizeof(context));
					context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
					if (!GetThreadContext(openThread, &context))
						return ERROR_GET_CONTEXT;

					freeReg = GetFreeReg(&context.Dr7);
					if (freeReg == -1)
						return ERROR_GET_FREE_REG;
					isRegDefined = true;
				}
				SetHook(source, destination, &openThread, &freeReg);
				CloseHandle(openThread);
			}
		} while (Thread32Next(hThread, &te32));
		CloseHandle(hThread);
		Unfreeze();

		auto record = info->GetItem(freeReg);
		record->source = source;
		record->destination = destination;
		record->pTrampoline = source;

		return SUCCESS_CODE;
	}

	int HardwareBPHook::SetHook(void* source, void* destination, HANDLE* hThread, int* reg) {
		CONTEXT context;
		ZeroMemory(&context, sizeof(context));
		context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		if (!GetThreadContext(*hThread, &context))
			return ERROR_GET_CONTEXT;

		*(&context.Dr0 + *reg) = (unsigned long long) source;
		context.Dr7 |= 1ULL << (2 * (*reg));
		context.Dr7 |= HW_EXECUTE << ((*reg) * 4 + 16);
		context.Dr7 |= HW_LENGTH << ((*reg) * 4 + 18);

		if (!SetThreadContext(*hThread, &context))
			return ERROR_SET_CONTEXT;
		return SUCCESS_CODE;
	}

	int HardwareBPHook::UnsetHook(void* source) {
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
		Freeze();
		do {
			if (te32.th32OwnerProcessID == dwOwnerPID) {
				HANDLE openThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
				UnsetHook(source, &openThread);
				CloseHandle(openThread);
			}
		} while (Thread32Next(hThread, &te32));
		CloseHandle(hThread);
		Unfreeze();
		return SUCCESS_CODE;
	}

	int HardwareBPHook::UnsetHook(void* source, HANDLE* hThread) {
		CONTEXT context;
		ZeroMemory(&context, sizeof(context));
		context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		if (!GetThreadContext(*hThread, &context))
			return ERROR_GET_CONTEXT;

		for (int i = 0; i < DEBUG_REG_COUNT; i++) {
			if ((unsigned long long)source == *(&context.Dr0 + i)) {
				info->GetItem(i)->source = 0;
				*(&context.Dr0 + i) = 0;
				context.Dr7 &= ~(1ULL << (2 * i));
				context.Dr7 &= ~(3 << (i * 4 + 16));
				context.Dr7 &= ~(3 << (i * 4 + 18));
				break;
			}
		}
		if (!SetThreadContext(*hThread, &context))
			return ERROR_SET_CONTEXT;

		return SUCCESS_CODE;
	}

	silk_data::Vector<HookRecord*>* HardwareBPHook::GetInfo() {
		return info;
	}

	HookRecord* HardwareBPHook::GetRecordBySource(void* source) {
		auto len = info->Length();
		for (int i = 0; i < len; i++)
			if (info->GetItem(i)->source == source)
				return info->GetItem(i);
		return nullptr;
	}
}