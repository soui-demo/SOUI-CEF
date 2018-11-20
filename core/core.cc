#include "stdafx.h"
#include "main_window.h"

#include "webview\webview.h"
#include "webview\client_app.h"
#include "webview\main_context_impl.h"

#include "IRunEngine.h"

//从PE文件加载，注意从文件加载路径位置
//#define RES_TYPE 0   //从文件中加载资源
// #define RES_TYPE 1  //从PE资源中加载UI资源
#if defined _DEBUG || defined DEBUG
#define RES_TYPE 1
#else
#define RES_TYPE 1
#endif

ROBJ_IN_CPP

namespace SOUI {

class SMsgLoopCef :public SMessageLoop {
public:
	virtual BOOL OnIdle(int nIdleCount) {
		browser::MainContext::DoMessageLoopWork();
		return __super::OnIdle(nIdleCount);
	}
};

class SMsgLoopFactory :public TObjRefImpl<IMsgLoopFactory> {
public:
	virtual SMessageLoop *CreateMsgLoop() {
		return new SMsgLoopCef;
	}

	virtual void DestoryMsgLoop(SMessageLoop *pMsgLoop) {
		delete pMsgLoop;
	}
};

}

class RunEngineImpl :public IRunEngine {
public:
	RunEngineImpl()
		:cef_ctx_(new browser::MainContextImpl(true)) {
		auto hRes = ::OleInitialize(nullptr);
		assert(SUCCEEDED(hRes));

		com_mgr_ = new SComMgr();

//#if defined DEBUG || defined _DEBUG
//		WCHAR szCurrentDir[MAX_PATH] = {0};
//		GetModuleFileNameW(NULL, szCurrentDir, sizeof(szCurrentDir));
//		LPWSTR lpInsertPos = wcsrchr(szCurrentDir, L'\\');
//		wcscpy(lpInsertPos + 1, L"..\\steambox");
//		SetCurrentDirectoryW(szCurrentDir);
//#endif
	}

	~RunEngineImpl() {

		::OleUninitialize();
	}

	//
	//IRunEngine methods
	//

	virtual BOOL Initialize(HINSTANCE hInstance) override {
		BOOL bLoaded = FALSE;
		CAutoRefPtr<IRenderFactory> pRenderFactory;
		bLoaded = com_mgr_->CreateRender_Skia((IObjRef**)&pRenderFactory);
		SASSERT_FMTW(bLoaded, L"load interface [render] failed!");

		CAutoRefPtr<IImgDecoderFactory> pImgDecoderFactory;
		bLoaded = com_mgr_->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
		SASSERT_FMTW(bLoaded, L"load interface [image decoder] falied!");
		pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);

		app_ = new SOUI::SApplication(pRenderFactory, hInstance, L"#10887");

		RegisterWindowClass();
		RegisterSkinClass();
		LoadResource(hInstance);
		InitializeCef(hInstance);

		CreateSingleton();

		return TRUE;
	}

	virtual int Run() override {
		int nRet = 0;
		steambox::SMainWnd main_wnd;
		main_wnd.Create(GetActiveWindow(), 0, 0, 0, 0);
		main_wnd.SendMessage(WM_INITDIALOG);
		main_wnd.CenterWindow(main_wnd.m_hWnd);
		main_wnd.ShowWindow(SW_SHOWNORMAL);
		nRet = app_->Run(main_wnd.m_hWnd);
		return nRet;
	}

	virtual void Release() override {
		ReleaseSingleton();

		cef_ctx_->Shutdown();
		cef_ctx_.reset();

		if (app_ != nullptr) {
			delete app_;
			app_ = nullptr;
		}
		delete com_mgr_;
	}

protected:
	void RegisterWindowClass() {
		app_->RegisterWindowClass<SWebView>();
	}

	void RegisterSkinClass() {
	}

	void LoadResource(HINSTANCE hInstance) {

		//加载系统资源
		{
			HMODULE hMod = LoadLibrary(_T("soui-sys-resource.dll"));
			CAutoRefPtr<IResProvider> sysResProvider;
			CreateResProvider(RES_PE, (IObjRef**) &sysResProvider);
			sysResProvider->Init((WPARAM)hMod, 0);
			app_->LoadSystemNamedResource(sysResProvider);
			FreeLibrary(hMod);
		}

		//加载盒子资源
		CAutoRefPtr<IResProvider> pResProvider;
#if (RES_TYPE == 0)
		CreateResProvider(RES_FILE, (IObjRef**) &pResProvider);
		if (!pResProvider->Init((LPARAM) _T("uires"), 0)) {
			SASSERT(0);
			return;
		}
#else
		CreateResProvider(RES_PE, (IObjRef**) &pResProvider);
		pResProvider->Init((WPARAM) hInstance, 0);
#endif
		app_->InitXmlNamedID(namedXmlID, ARRAYSIZE(namedXmlID), TRUE);
		app_->AddResProvider(pResProvider);
	}

	void InitializeCef(HINSTANCE hInstance) {
		IMsgLoopFactory *pMsgLoopFactory = new SMsgLoopFactory;
		app_->SetMsgLoopFactory(pMsgLoopFactory);
		pMsgLoopFactory->Release();

		CefMainArgs main_args(hInstance);
		CefRefPtr<CefApp> cef = browser::MainContext::InitCef3(main_args, nullptr);
		assert(cef.get() != nullptr);
		
		cef_ctx_->Initialize(main_args, cef, nullptr);
	}

	void CreateSingleton() {
		new SNotifyCenter();
	}

	void ReleaseSingleton() {
		delete SNotifyCenter::getSingletonPtr();

	}

private:
	SApplication *app_ = nullptr;
	SComMgr *com_mgr_ = nullptr;
	scoped_ptr<browser::MainContextImpl> cef_ctx_;
};


extern "C" __declspec(dllexport) IRunEngine *CreateInterface(const WCHAR *iface_version) {
	return new RunEngineImpl();
}