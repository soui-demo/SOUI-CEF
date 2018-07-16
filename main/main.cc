// updater.cpp: 定义应用程序的入口点。
//

#include <windows.h>
#include <stdio.h>
#include <string>
#include <assert.h>

static const WCHAR kCore[] = L"core.dll";

class IRunEngine {
public:
	virtual BOOL Initialize(HINSTANCE hInstance) = 0;
	virtual int Run() = 0;
	virtual void Release() = 0;
};


typedef IRunEngine *(*pfnCreateInterface)(const WCHAR*);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					  _In_opt_ HINSTANCE hPrevInstance,
					  _In_ LPWSTR    lpCmdLine,
					  _In_ int       nCmdShow)
{
	WCHAR szCurrentDir[MAX_PATH] = {0};
	GetModuleFileNameW(NULL, szCurrentDir, sizeof(szCurrentDir));
	LPWSTR lpInsertPos = wcsrchr(szCurrentDir, L'\\');
	*(++lpInsertPos) = 0;
	SetCurrentDirectoryW(szCurrentDir);
	auto lib = LoadLibraryW(kCore);
	if (lib) {
		auto fn = (pfnCreateInterface) GetProcAddress(lib, "CreateInterface");
		auto engine = fn(L"SOUI");
		engine->Initialize(lib);
		auto nRet = engine->Run();
		engine->Release();

		FreeLibrary(lib);
		return nRet;
	}

	return 0;
}