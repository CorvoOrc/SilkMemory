#include "OpcodeHook.h"
#include "MemoryScan.h"

using namespace std;

namespace silk_way {
	ULONG_PTR FindPrev(ULONG_PTR origin, ULONG_PTR minAddr, DWORD step, int size);
	ULONG_PTR FindNext(ULONG_PTR origin, ULONG_PTR maxAddr, DWORD step, int size);

	void OpcodeHook::SetExceptionHandler(PVECTORED_EXCEPTION_HANDLER pVecExcHandler) { }

	int OpcodeHook::SetHook(void *source, void *destination) {
		auto record = new HookRecord();
		record->source = source;
		record->destination = destination;

		info->PushBack(record);

		JMP_ABS pattern = { 0xFF, 0x25, 0x00000000,    // JMP[RIP + 6] empty
							0x0000000000000000 };      // address
		pattern.address = (ULONG_PTR)source;
		
		int currentLen = 0;
		bool isJmpOpcode = false;
		int redLine = sizeof(JMP_REL);
		while (currentLen < redLine && !isJmpOpcode) {
			hde64s context;
			const void* pSource = (void*)((unsigned char*)source + currentLen);
			hde64_disasm(pSource, &context);

			if (context.opcode == 0xE9) {
				ULONG_PTR ripPtr = (ULONG_PTR)pSource + context.len + (INT32)context.imm.imm32;
				pattern.address = ripPtr;
				isJmpOpcode = true;
			}

			memcpy((unsigned char*)record->sourceReservation + currentLen, pSource, context.len);
			record->reservationLen += context.len;
			currentLen += context.len;
		}


		int trampolineMemorySize = isJmpOpcode ? 2 * sizeof(JMP_ABS) : 2 * sizeof(JMP_ABS) + record->reservationLen;
		record->pTrampoline = AllocateMemory(source, trampolineMemorySize);
		if (!isJmpOpcode) {
			pattern.address = (unsigned long long)(unsigned char*)source + record->reservationLen;
			memcpy((unsigned char*)record->pTrampoline, record->sourceReservation, record->reservationLen);
		}
		int offset = isJmpOpcode ? 0 : record->reservationLen;
		memcpy((unsigned char*)record->pTrampoline + offset, &pattern, sizeof(JMP_ABS));

		pattern.address = (ULONG_PTR)destination;
		ULONG_PTR relay = (ULONG_PTR)record->pTrampoline + sizeof(pattern) + record->reservationLen;
		memcpy((void*)relay, &pattern, sizeof(pattern));

		DWORD oldProtect = 0;
		VirtualProtect(source, sizeof(JMP_REL), PAGE_EXECUTE_READWRITE, &oldProtect);

		JMP_REL* pJmpRelPattern = (JMP_REL*)source;
		pJmpRelPattern->opcode = 0xE9;
		pJmpRelPattern->delta = (unsigned int)((LPBYTE)relay - ((LPBYTE)source + sizeof(JMP_REL)));

		VirtualProtect(source, sizeof(JMP_REL), oldProtect, &oldProtect);

		return SUCCESS_CODE;
	}

	int OpcodeHook::UnsetHook(void *source) {
		auto record = GetRecordBySource(source);
		DWORD oldProtect = 0;
		VirtualProtect(source, sizeof(JMP_REL), PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(source, record->sourceReservation, record->reservationLen);
		VirtualProtect(source, sizeof(JMP_REL), oldProtect, &oldProtect);
		info->Erase(record);
		FreeMemory(record);
		return SUCCESS_CODE;
	}

	void* OpcodeHook::AllocateMemory(void* origin, int size) {
		const unsigned int MEMORY_RANGE = 0x40000000;
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		ULONG_PTR minAddr = (ULONG_PTR)sysInfo.lpMinimumApplicationAddress;
		ULONG_PTR maxAddr = (ULONG_PTR)sysInfo.lpMaximumApplicationAddress;

		ULONG_PTR castedOrigin = (ULONG_PTR)origin;
		ULONG_PTR minDesired = castedOrigin - MEMORY_RANGE;
		if (minDesired > minAddr && minDesired < castedOrigin)
			minAddr = minDesired;
		int test = sizeof(ULONG_PTR);
		ULONG_PTR maxDesired = castedOrigin + MEMORY_RANGE - size;
		if (maxDesired < maxAddr && maxDesired > castedOrigin)
			maxAddr = maxDesired;

		DWORD granularity = sysInfo.dwAllocationGranularity;
		ULONG_PTR freeMemory = 0;
		ULONG_PTR ptr = castedOrigin;
		while (ptr >= minAddr) {
			ptr = FindPrev(ptr, minAddr, granularity, size);
			if (ptr == 0)
				break;
			LPVOID pAlloc = VirtualAlloc((LPVOID)ptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (pAlloc != 0)
				return pAlloc;
		}
		while (ptr < maxAddr) {
			ptr = FindNext(ptr, maxAddr, granularity, size);
			if (ptr == 0)
				break;
			LPVOID pAlloc = VirtualAlloc((LPVOID)ptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (pAlloc != 0)
				return pAlloc;
		}
		return NULL;
	}

	void OpcodeHook::FreeMemory(HookRecord* record) {
		VirtualFree(record->pTrampoline, 0, MEM_RELEASE);
		delete record;
	}

	ULONG_PTR FindPrev(ULONG_PTR origin, ULONG_PTR minAddr, DWORD step, int size) {
		ULONG_PTR p = origin - origin % step;
		p -= step;
		while (p >= minAddr)
		{
			MEMORY_BASIC_INFORMATION info;
			int test = sizeof(info);
			if (!VirtualQuery((LPVOID)p, &info, sizeof(info)))
				return NULL;
			if (info.State == MEM_FREE)
				return p;
			p = (ULONG_PTR)info.AllocationBase - step;
		}
		return NULL;
	}

	ULONG_PTR FindNext(ULONG_PTR origin, ULONG_PTR maxAddr, DWORD step, int size) {
		ULONG_PTR p = origin - origin % step;
		p += step;
		while (p <= maxAddr)
		{
			MEMORY_BASIC_INFORMATION info;
			if (!VirtualQuery((LPCVOID)p, &info, sizeof(info)))
				return NULL;
			if (info.State == MEM_FREE)
				return p;
			p = (ULONG_PTR)info.BaseAddress + info.RegionSize + step - 1;
			p -= p % step;
		}
		return NULL;
	}

	silk_data::Vector<HookRecord*>* OpcodeHook::GetInfo() {
		return info;
	}

	HookRecord* OpcodeHook::GetRecordBySource(void* source) {
		auto len = info->Length();
		for (int i = 0; i < len; i++)
			if (info->GetItem(i)->source == source)
				return info->GetItem(i);
		return nullptr;
	}
}