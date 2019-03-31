#pragma once
#include "Windows.h"
#include "Tlhelp32.h"
#include <iostream>
#include "HardwareBPHook.h"
#include "DeferredCommand.h"
#include "MemoryScan.h"
#include "OpcodeHook.h"

using namespace std;
using namespace silk_way;
using namespace silk_data;

class Unknown {
public:
	Unknown() { }
	~Unknown() { }
	virtual HRESULT QueryInterface() = 0;
	virtual ULONG AddRef(void) = 0;
	virtual ULONG Release(void) = 0;
};
class Device : public Unknown {
public:
	Device() : Unknown() {

	}
	~Device() {

	}
	virtual HRESULT QueryInterface() {
		return 0;
	}
	virtual ULONG AddRef(void) {
		return 0;
	}
	virtual ULONG Release(void) {
		return 0;
	}
	virtual int Present() {
		cout << "Present()" << " " << i << endl;
		return i;
	}
	virtual void EndScene(int j) {
		cout << "EndScene()" << " " << i << " " << j << endl;
	}
	void Dispose() {
		cout << "Dispose()" << " " << i << endl;
	}
public:
	int i;
};

IDeferredCommands* hookCommands;
IHook* hook;

typedef int(*pPresent)(Device*);
typedef void(*pEndScene)(Device*, int j);

pPresent ptrPresent = nullptr;
pEndScene ptrEndScene = nullptr;

int PresentHook(Device* device) {
	auto record = hook->GetRecordBySource(ptrPresent);
	pPresent pTrampoline = (pPresent)record->pTrampoline;
	auto result = pTrampoline(device);
	cout << "PresentHook" << endl;
	hookCommands->Run();
	return result;
}

void EndSceneHook(Device* device, int j) {
	auto record = hook->GetRecordBySource(ptrEndScene);
	pEndScene pTrampoline = (pEndScene)record->pTrampoline;
	pTrampoline(device, 2);
	cout << "EndSceneHook" << " " << j << endl;
	hookCommands->Run();
}

LONG OnExceptionHandler(EXCEPTION_POINTERS* exceptionPointers) {
	if (exceptionPointers->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP)
		return EXCEPTION_CONTINUE_EXECUTION;
	for (int i = 0; i < DEBUG_REG_COUNT; i++) {
		if (exceptionPointers->ContextRecord->Rip == (unsigned long long) hook->GetInfo()->GetItem(i)->source) {
			exceptionPointers->ContextRecord->Dr7 &= ~(1ULL << (2 * i));
			exceptionPointers->ContextRecord->Rip = (unsigned long long) hook->GetInfo()->GetItem(i)->destination;
			IDeferredCommand* cmd = new SetD7Command(hook, GetCurrentThreadId(), i);
			hookCommands->Enqueue(cmd);
			break;
		}
	}
	return EXCEPTION_CONTINUE_EXECUTION;
}

int main() {
	while (true) {
		Device* device = new Device();
		device->i = 3;
		unsigned long long vmt = **(unsigned long long **)&device;
		ptrPresent = (pPresent)(*(unsigned long long*)(vmt + 8 * 3));
		ptrEndScene = (pEndScene)(*(unsigned long long*)(vmt + 8 * 4));

		IScanner* scan = new ConsoleScanner();
		scan->PrintMemory("Present", (unsigned char*)ptrPresent, 30);

		hookCommands = new DeferredCommands();

		//hook = new HardwareBPHook();
		hook = new OpcodeHook();
		hook->SetExceptionHandler(OnExceptionHandler);

		hook->SetHook(ptrPresent, &PresentHook);
		hook->SetHook(ptrEndScene, &EndSceneHook);

		device->Present();
		device->EndScene(7);
		device->Present();
		device->EndScene(7);
		device->i = 5;
		ptrPresent(device);
		ptrEndScene(device, 9);

		hook->UnsetHook(ptrPresent);
		hook->UnsetHook(ptrEndScene);
		ptrPresent(device);
		ptrEndScene(device, 7);

		delete hookCommands;
		delete hook;
		delete device;
	}
}
