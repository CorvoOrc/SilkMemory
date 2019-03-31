#include <Windows.h>
#include <TlHelp32.h>
#include <fstream>
#include "SilkWay/Controller.h"
#include "DeferredCommand.h"
#include "HardwareBPHook.h"
#include "OpcodeHook.h"
#include "MemoryScan.h"

DWORD WINAPI HookThread(LPVOID lpParam);
BOOL CreateSearchDevice(IDirect3D9** d3d, IDirect3DDevice9** device);
BOOL HookDevice(IDirect3DDevice9* pDevice);
void Dispose();

typedef IDirect3D9* (WINAPI *SILK_Direct3DCreate9) (UINT SDKVersion);

typedef HRESULT(WINAPI *SILK_GetAdapterDisplayMode)(IDirect3D9* direct3D9, UINT Adapter, D3DDISPLAYMODE* pMode);
typedef HRESULT(WINAPI *SILK_CreateDevice)(IDirect3D9* direct3D9, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface);
typedef ULONG(WINAPI *SILK_Release)(IDirect3D9* direct3D9);
const int RELEASE_INDEX = 2;
const int GET_ADAPTER_DISPLAY_MODE_INDEX = 8;
const int CREATE_DEVICE_INDEX = 16;

typedef HRESULT(*VirtualOverloadPresent)(IDirect3DDevice9* pd3dDevice, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
VirtualOverloadPresent oOverload = NULL;
typedef HRESULT(*VirtualOverloadEndScene)(IDirect3DDevice9* pd3dDevice);
VirtualOverloadEndScene oOverloadEndScene = NULL;
const int PRESENT_INDEX = 17;
const int END_SCENE_INDEX = 42;

silk_way::IDeferredCommands* deferredCommands;
silk_way::IHook* hook;
IScanner* scanner = nullptr;
ILogger* logger = nullptr;
IController* controller = nullptr;

const int PATH_LEN = 128;
char* dllPath;
char* screenshotPath;
char* scannerPath;
char* loggerPath;

void InitPath(HMODULE* module);

typedef bool(*fDoIncludeScript)(const char*, unsigned long long);

extern "C" __declspec(dllexport) bool WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved) {
	DisableThreadLibraryCalls((HMODULE)hInstDll);
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH: {
			InitPath(&hInstDll);
			logger = new MemoryLogger(loggerPath);
			scanner = new FileScanner(scannerPath);
			CreateThread(0, 0, HookThread, 0, 0, 0);
			break;
		}
		case DLL_PROCESS_DETACH: {
			Dispose();
			break;
		}
	}
	return TRUE;
}

void InitPath(HMODULE* module) {
	dllPath = new char[PATH_LEN];
	GetModuleFileName(*module, dllPath, PATH_LEN);
	char* pch = strrchr(dllPath, '\\');
	*(pch + 1) = '\0';

	screenshotPath = new char[PATH_LEN];
	strcat(screenshotPath, dllPath);
	strcat(screenshotPath, "screenshot.bmp");

	scannerPath = new char[PATH_LEN];
	strcat(scannerPath, dllPath);
	strcat(scannerPath, "scan.scan");

	loggerPath = new char[PATH_LEN];
	strcat(loggerPath, dllPath);
	strcat(loggerPath, "logger.log");
}

DWORD WINAPI HookThread(LPVOID lpParam) {
	IDirect3D9* pSearchD3D = NULL;
	IDirect3DDevice9* pSearchDevice = NULL;

	while (CreateSearchDevice(&pSearchD3D, &pSearchDevice) == FALSE)
		Sleep(1000);

	if (pSearchD3D)
		pSearchD3D->Release();

	if (!HookDevice(pSearchDevice))
		return 1;

	if (pSearchDevice)
		pSearchDevice->Release();

	return 0;
}

BOOL CreateSearchDevice(IDirect3D9** d3d, IDirect3DDevice9** device) {
	if (!d3d || !device)
		return FALSE;

	*d3d = NULL;
	*device = NULL;

	//IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	SILK_Direct3DCreate9 Silk_Direct3DCreate9 = (SILK_Direct3DCreate9)GetProcAddress(GetModuleHandle("d3d9.dll"), "Direct3DCreate9");
	IDirect3D9* pD3D = Silk_Direct3DCreate9(D3D_SDK_VERSION);

	if (!pD3D)
		return FALSE;

	D3DDISPLAYMODE displayMode;
	int pointerSize = sizeof(unsigned long long);
	unsigned long long vmt = **(unsigned long long **)&pD3D;
	SILK_GetAdapterDisplayMode pGetAdapderDisplayMode = (SILK_GetAdapterDisplayMode)((*(unsigned long long *)(vmt + pointerSize * GET_ADAPTER_DISPLAY_MODE_INDEX)));
	pGetAdapderDisplayMode(pD3D, D3DADAPTER_DEFAULT, &displayMode);
	//pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);

	HWND hWindow = GetDesktopWindow();

	D3DPRESENT_PARAMETERS pp;
	ZeroMemory(&pp, sizeof(pp));
	pp.Windowed = TRUE;
	pp.hDeviceWindow = hWindow;
	pp.BackBufferCount = 0;
	pp.BackBufferWidth = 0;
	pp.BackBufferHeight = 0;
	pp.BackBufferFormat = displayMode.Format;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;

	IDirect3DDevice9* pDevice = NULL;

	SILK_CreateDevice pCreateDevice = (SILK_CreateDevice)((*(unsigned long long *)(vmt + pointerSize * CREATE_DEVICE_INDEX)));
	if(SUCCEEDED(pCreateDevice(pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &pp, &pDevice))) {
	//if (SUCCEEDED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &pp, &pDevice))) {
		if (pDevice != NULL) {
			*d3d = pD3D;
			*device = pDevice;
		}
	}

	BOOL result = (*d3d != NULL);
	if (result == FALSE)
		if (pD3D) {
			SILK_Release pRelease = (SILK_Release)((*(unsigned long long *)(vmt + pointerSize * RELEASE_INDEX)));
			pRelease(pD3D);
			//pD3D->Release();
		}

	return result;
}

VOID WINAPI Capture(IDirect3DDevice9* pd3dDevice) {
	IDirect3DSurface9 *renderTarget = NULL;
	IDirect3DSurface9 *destTarget = NULL;
	HRESULT res1 = pd3dDevice->GetRenderTarget(0, &renderTarget);
	D3DSURFACE_DESC descr;
	HRESULT res2 = renderTarget->GetDesc(&descr);
	HRESULT res3 = pd3dDevice->CreateOffscreenPlainSurface(descr.Width, descr.Height, /*D3DFMT_A8R8G8B8*/descr.Format, D3DPOOL_SYSTEMMEM, &destTarget, NULL);
	HRESULT res4 = pd3dDevice->GetRenderTargetData(renderTarget, destTarget);
	D3DLOCKED_RECT lockedRect;
	ZeroMemory(&lockedRect, sizeof(lockedRect));
	if (destTarget == NULL)
		return;

	HRESULT res5 = destTarget->LockRect(&lockedRect, NULL, D3DLOCK_READONLY);

	HRESULT res7 = destTarget->UnlockRect();
	HRESULT res6 = D3DXSaveSurfaceToFile(screenshotPath, D3DXIFF_BMP, destTarget, NULL, NULL);
	renderTarget->Release();
	destTarget->Release();
}

void Dispose() {
	hook->UnsetHook(oOverload);
	hook->UnsetHook(oOverloadEndScene);
	delete scanner;
	delete logger;
	delete controller;
	delete deferredCommands;
	delete hook;
}

HRESULT WINAPI PresentHook(IDirect3DDevice9* pd3dDevice, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
	Capture(pd3dDevice);

	auto record = hook->GetRecordBySource(oOverload);
	VirtualOverloadPresent pTrampoline = (VirtualOverloadPresent)record->pTrampoline;
	auto result = pTrampoline(pd3dDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

	deferredCommands->Run();
	return result;
}

HRESULT WINAPI EndSceneHook(IDirect3DDevice9* pd3dDevice) {
	if (controller == nullptr) {
		controller = new GameController(logger);
		controller->SetDevice(pd3dDevice);
		fDoIncludeScript DoIncludeScript = (fDoIncludeScript)scanner->FindPattern("client.dll",
			"40 57 48 81 EC ?? ?? ?? ?? 48 83 3D ?? ?? ?? ?? ?? 48 8B F9 0F 84");
		DoIncludeScript("silk_way", 0);
	}
	controller->Update();

	auto record = hook->GetRecordBySource(oOverloadEndScene);
	VirtualOverloadEndScene pTrampoline = (VirtualOverloadEndScene)record->pTrampoline;
	auto result = pTrampoline(pd3dDevice);

	deferredCommands->Run();
	return result;
}

LONG OnExceptionHandler(EXCEPTION_POINTERS* exceptionPointers) {
	if (exceptionPointers->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP)
		return EXCEPTION_EXIT_UNWIND;

	for (int i = 0; i < silk_way::DEBUG_REG_COUNT; i++) {
		if (exceptionPointers->ContextRecord->Rip == (unsigned long long) hook->GetInfo()->GetItem(i)->source) {
			exceptionPointers->ContextRecord->Dr7 &= ~(1ULL << (2 * i));
			exceptionPointers->ContextRecord->Rip = (unsigned long long) hook->GetInfo()->GetItem(i)->destination;
			silk_way::IDeferredCommand* cmd = new silk_way::SetD7Command(hook, GetCurrentThreadId(), i);
			deferredCommands->Enqueue(cmd);
			break;
		}
	}
	return EXCEPTION_CONTINUE_EXECUTION;
}

BOOL HookDevice(IDirect3DDevice9* pDevice) {
	unsigned long long vmt = **(unsigned long long **)&pDevice;
	int pointerSize = sizeof(unsigned long long);
	VirtualOverloadPresent pointerPresent = (VirtualOverloadPresent)((*(unsigned long long *)(vmt + pointerSize * PRESENT_INDEX)));
	VirtualOverloadEndScene pointerEndScene = (VirtualOverloadEndScene)((*(unsigned long long *)(vmt + pointerSize * END_SCENE_INDEX)));
	oOverload = pointerPresent;
	oOverloadEndScene = pointerEndScene;
	deferredCommands = new silk_way::DeferredCommands();
	//hook = new silk_way::HardwareBPHook();
	hook = new silk_way::OpcodeHook();
	hook->SetExceptionHandler(OnExceptionHandler);
	hook->SetHook(pointerPresent, &PresentHook);
	hook->SetHook(pointerEndScene, &EndSceneHook);
	return TRUE;
}