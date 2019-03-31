#include "Hook.h"

namespace silk_way {
	const int DEBUG_REG_COUNT = 4;

	const int HW_EXECUTE = 0;
	const int HW_READ = 1;
	const int HW_WRITE = 2;
	const int HW_ACCESS = 3;

	const int HW_LENGTH = 0;

	class HardwareBPHook : public IHook {
	public:
		HardwareBPHook() : IHook() {
			info = new silk_data::Vector<HookRecord*>();
			for (int i = 0; i < DEBUG_REG_COUNT; i++) {
				auto record = new HookRecord();
				info->PushBack(record);
			}
		}
		~HardwareBPHook() {
			info->Clear();
			delete info;
			RemoveVectoredExceptionHandler(pException);
		}
		void SetExceptionHandler(PVECTORED_EXCEPTION_HANDLER pVecExcHandler);
		int SetHook(void* source, void* destination);
		int UnsetHook(void* source);
		silk_data::Vector<HookRecord*>* GetInfo();
		HookRecord* GetRecordBySource(void* source);
	private:
		int SetHook(void* source, void* destination, HANDLE* hThread, int* reg);
		int UnsetHook(void* source, HANDLE* hThread);
		int GetFreeReg(unsigned long long* mask);
	private:
		silk_data::Vector<HookRecord*>* info;
		PVOID pException;
	};
}