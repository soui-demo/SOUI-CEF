#include "include/cef_client.h"
#include "client_renderer_app.h"

#if defined DEBUG || defined _DEBUG
const char kSubProcess[] = "steamboxprocessd.exe";
#else
const char kSubProcess[] = "steamboxprocess.exe";
#endif

void RenderProcessClientApp::OnBeforeCommandLineProcessing(
	const CefString& process_type,
	CefRefPtr<CefCommandLine> command_line) {
	// Pass additional command-line flags to the browser process.
	if (process_type.empty()) {

		// Pass additional command-line flags when off-screen rendering is enabled.

		// If the PDF extension is enabled then cc Surfaces must be disabled for
		// PDFs to render correctly.
		// See https://bitbucket.org/chromiumembedded/cef/issues/1689 for details.
		if (!command_line->HasSwitch("disable-extensions") &&
			!command_line->HasSwitch("disable-pdf-extension")) {
			command_line->AppendSwitch("disable-surfaces");
		}

		// Use software rendering and compositing (disable GPU) for increased FPS
		// and decreased CPU usage. This will also disable WebGL so remove these
		// switches if you need that capability.
		// See https://bitbucket.org/chromiumembedded/cef/issues/1257 for details.
		command_line->AppendSwitch("disable-gpu");
		command_line->AppendSwitch("disable-gpu-compositing");

		// Synchronize the frame rate between all processes. This results in
		// decreased CPU usage by avoiding the generation of extra frames that
		// would otherwise be discarded. The frame rate can be set at browser
		// creation time via CefBrowserSettings.windowless_frame_rate or changed
		// dynamically using CefBrowserHost::SetWindowlessFrameRate. In cefclient
		// it can be set via the command-line using `--off-screen-frame-rate=XX`.
		// See https://bitbucket.org/chromiumembedded/cef/issues/1368 for details.
		command_line->AppendSwitch("enable-begin-frame-scheduling");

		command_line->AppendSwitch("--ignore-certificate-errors");
		command_line->AppendSwitch("--no-proxy-server");

		// 此参数解决多窗口问题
		command_line->AppendSwitch("process-per-site");
		//command_line->AppendSwitch("enable-npapi");
		//command_line->AppendSwitchWithValue("register-pepper-plugins", "PepperFlash/pepflashplayer.dll;application/x-shockwave-flash");

		//command_line->AppendSwitchWithValue("ppapi-flash-version", "22.0.0.209");//PepperFlash\manifest.json中的version
		//command_line->AppendSwitchWithValue("ppapi-flash-path", "PepperFlash\\pepflashplayer.dll");


		command_line->AppendSwitchWithValue("--renderer-process-limit", "1");
		command_line->AppendSwitchWithValue("--browser-subprocess-path", kSubProcess);
	}

}

CefRefPtr<RenderProcessClientApp>& RenderProcessClientApp::Instance() {
	static CefRefPtr<RenderProcessClientApp> pApp;
	if (pApp == NULL) {
		pApp = new RenderProcessClientApp();
	}

	return pApp;
}

void RenderProcessClientApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
										   CefRefPtr<CefFrame> frame,
										   CefRefPtr<CefV8Context> context) {
	//CefRefPtr<CefV8Value> object = context->GetGlobal();

	//CefRefPtr<CefV8Handler> handler = new HtmlEventHandler();
	//CefRefPtr<CefV8Value> funcCallClient = CefV8Value::CreateFunction("CallClient", handler);

	//object->SetValue("CallClient", funcCallClient, V8_PROPERTY_ATTRIBUTE_NONE);
}

void RenderProcessClientApp::OnUncaughtException(CefRefPtr<CefBrowser> browser,
											  CefRefPtr<CefFrame> frame,
											  CefRefPtr<CefV8Context> context,
											  CefRefPtr<CefV8Exception> exception,
											  CefRefPtr<CefV8StackTrace> stackTrace) {
}

bool RenderProcessClientApp::OnProcessMessageReceived(
	CefRefPtr<CefBrowser> browser,
	CefProcessId source_process,
	CefRefPtr<CefProcessMessage> message) {
	return true;
}
