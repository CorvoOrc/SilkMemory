#pragma once
#include "Hook.h"
#include "hde64.h"

namespace silk_way {
	class OpcodeHook : public IHook {
	public:
		OpcodeHook() : IHook() {
			info = new silk_data::Vector<HookRecord*>();
		}
		~OpcodeHook() {
			info->Clear();
			delete info;
		}
		void SetExceptionHandler(PVECTORED_EXCEPTION_HANDLER pVecExcHandler);
		int SetHook(void* source, void* destination);
		int UnsetHook(void* source);
		silk_data::Vector<HookRecord*>* GetInfo();
		HookRecord* GetRecordBySource(void* source);
	private:
		void* AllocateMemory(void* origin, int size);
		void FreeMemory(HookRecord* record);
	private:
		silk_data::Vector<HookRecord*>* info;
	};
}