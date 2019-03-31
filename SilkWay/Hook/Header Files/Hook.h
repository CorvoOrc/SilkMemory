#pragma once
#include "Windows.h"
#include "Tlhelp32.h"
#include "Vector.h"

namespace silk_way {
	const int RESERV_SIZE = 28;

	const int SUCCESS_CODE = 0;
	const int ERROR_ENUM_THREAD_START = 1 << 0;
	const int ERROR_GET_CONTEXT = 1 << 1;
	const int ERROR_SET_CONTEXT = 1 << 2;
	const int ERROR_GET_FREE_REG = 1 << 3;

	#pragma pack(push, 1)

	struct HookRecord {
		HookRecord() {
			reservationLen = 0;
			sourceReservation = new void*[RESERV_SIZE]();
		}
		~HookRecord() {
			reservationLen = 0;
			delete[] sourceReservation;
		}
		void* source;
		void* destination;
		void* pTrampoline;
		int reservationLen;
		void* sourceReservation;
	};

	// 32-bit relative jump/call.
	typedef struct {
		unsigned char opcode;
		unsigned int delta;
	} JMP_REL;

	// 64-bit absolute jump.
	typedef struct {
		unsigned char opcode1;
		unsigned char opcode2;
		unsigned int dummy;
		unsigned long long address;
	} JMP_ABS;

	#pragma pack(pop)

	class IHook {
	protected:
		IHook() { }
	public:
		virtual ~IHook() { }
		virtual void SetExceptionHandler(PVECTORED_EXCEPTION_HANDLER pVecExcHandler) = 0;
		virtual int SetHook(void* source, void* destination) = 0;
		virtual int UnsetHook(void* source) = 0;
		virtual silk_data::Vector<HookRecord*>* GetInfo() = 0;
		virtual HookRecord* GetRecordBySource(void* source) = 0;

		virtual int Freeze();
		virtual int Unfreeze();
	};
}