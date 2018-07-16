#ifndef _SUB_PROCESS_CLIENT_APP_H
#define _SUB_PROCESS_CLIENT_APP_H

#pragma once

#include "include/cef_app.h"

class CefClient;

class RenderProcessClientApp : public CefApp, public CefRenderProcessHandler {
public:

	RenderProcessClientApp() {
	}

	~RenderProcessClientApp() {
	}

	static CefRefPtr<RenderProcessClientApp>& Instance();


	virtual void OnBeforeCommandLineProcessing(
		const CefString& process_type,
		CefRefPtr<CefCommandLine> command_line) OVERRIDE;

	// CefApp methods:
	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE {
		return this;
	}

	//
	// CefRenderProcessHandler methods:
	//
	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
								  CefRefPtr<CefFrame> frame,
								  CefRefPtr<CefV8Context> context) OVERRIDE;

	virtual void OnUncaughtException(CefRefPtr<CefBrowser> browser,
									 CefRefPtr<CefFrame> frame,
									 CefRefPtr<CefV8Context> context,
									 CefRefPtr<CefV8Exception> exception,
									 CefRefPtr<CefV8StackTrace> stackTrace) OVERRIDE;

	virtual bool OnProcessMessageReceived(
		CefRefPtr<CefBrowser> browser,
		CefProcessId source_process,
		CefRefPtr<CefProcessMessage> message) OVERRIDE;

private:
	IMPLEMENT_REFCOUNTING(RenderProcessClientApp);
};


#endif //_SUB_PROCESS_CLIENT_APP_H