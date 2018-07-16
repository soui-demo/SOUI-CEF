#ifndef _BROWSER_MAIN_CONTEXT_H_
#define _BROWSER_MAIN_CONTEXT_H_
#pragma once

#include <string>
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/base/cef_ref_counted.h"
#include "include/internal/cef_types_wrappers.h"

namespace browser {
struct IBrowserClient {
	virtual CefClient *getClient() = 0;
	virtual HWND getWindowHandle() = 0;
};

// Used to store global context in the browser process. The methods of this
// class are thread-safe unless otherwise indicated.
class MainContext {
public:
	// Returns the singleton instance of this object.
	static MainContext* Get();

	static void DoMessageLoopWork();
	static CefRefPtr<CefApp> InitCef3(const CefMainArgs& args, void* windows_sandbox_info);

	// set dev tools client
	virtual void SetDevToolsClient(IBrowserClient *pBrowserClient) = 0;

	// Returns dev tools client
	virtual IBrowserClient *GetDevToolsClient() = 0;

	// Returns the full path to the console log file.
	virtual std::string GetConsoleLogPath() = 0;

	// Returns the full path to |file_name|.
	virtual std::string GetDownloadPath(const std::string& file_name) = 0;

	// Returns the app working directory including trailing path separator.
	virtual std::string GetAppWorkingDirectory() = 0;

	// Returns the main application URL.
	virtual std::string GetMainURL() = 0;

	//return the opened browser count
	virtual int GetOpenedBrowserCount() = 0;

	virtual int  AddOpenedBrowserCount() = 0;

	virtual int  DelOpenedBrowserCount() = 0;

protected:
	MainContext();
	virtual ~MainContext();

private:
	DISALLOW_COPY_AND_ASSIGN(MainContext);
};

}  // namespace browser

#endif  // _BROWSER_MAIN_CONTEXT_H_
