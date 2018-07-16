#include "client_app_browser.h"

#include "include/base/cef_logging.h"
#include "include/cef_cookie.h" 

namespace browser
{

namespace {

#if defined DEBUG || defined _DEBUG
const char kSubProcess[] = "webprocessd.exe";
#else
const char kSubProcess[] = "webprocess.exe";
#endif

}

ClientAppBrowser::ClientAppBrowser()
{
  CreateDelegates(delegates_);
} 

void ClientAppBrowser::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{ 
	if (process_type.empty())
	{ 
		command_line->AppendSwitch("--disable-gpu");
		command_line->AppendSwitch("--ignore-certificate-errors");
		command_line->AppendSwitch("--no-proxy-server");
		command_line->AppendSwitchWithValue("--renderer-process-limit", "1");
		command_line->AppendSwitchWithValue("--browser-subprocess-path", kSubProcess);
		command_line->AppendSwitch("--enable-media-stream");

		command_line->AppendSwitch("process-per-site");
		//command_line->AppendSwitchWithValue("ppapi-flash-version", "22.0.0.209");//PepperFlash\manifest.json中的version
		//command_line->AppendSwitchWithValue("ppapi-flash-path", "PepperFlash\\pepflashplayer.dll");

		 
		DelegateSet::iterator it = delegates_.begin();
		for (; it != delegates_.end(); ++it)
			(*it)->OnBeforeCommandLineProcessing(this, command_line);
	}

	/*
  // Pass additional command-line flags to the browser process.
  //if (process_type.empty())
  { 

    // Pass additional command-line flags when off-screen rendering is enabled.
     
    // If the PDF extension is enabled then cc Surfaces must be disabled for
    // PDFs to render correctly.
    // See https://bitbucket.org/chromiumembedded/cef/issues/1689 for details.
    if (!command_line->HasSwitch("disable-extensions") &&
        !command_line->HasSwitch("disable-pdf-extension"))
    {
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
    //command_line->AppendSwitch("enable-npapi");
    //command_line->AppendSwitchWithValue("register-pepper-plugins", "PepperFlash/pepflashplayer.dll;application/x-shockwave-flash");

	

  }
  */
}

void ClientAppBrowser::OnContextInitialized()
{
  // Register cookieable schemes with the global cookie manager.
  CefRefPtr<CefCookieManager> manager =
    CefCookieManager::GetGlobalManager(NULL);
  DCHECK(manager.get());
  manager->SetSupportedSchemes(cookieable_schemes_, NULL);

  print_handler_ = CreatePrintHandler();

  DelegateSet::iterator it = delegates_.begin();
  for (; it != delegates_.end(); ++it)
    (*it)->OnContextInitialized(this);
}

void ClientAppBrowser::OnBeforeChildProcessLaunch(
  CefRefPtr<CefCommandLine> command_line)
{
  DelegateSet::iterator it = delegates_.begin();
  for (; it != delegates_.end(); ++it)
    (*it)->OnBeforeChildProcessLaunch(this, command_line);
}

void ClientAppBrowser::OnRenderProcessThreadCreated(
  CefRefPtr<CefListValue> extra_info)
{
  DelegateSet::iterator it = delegates_.begin();
  for (; it != delegates_.end(); ++it)
    (*it)->OnRenderProcessThreadCreated(this, extra_info);
}


// static
void ClientAppBrowser::CreateDelegates(DelegateSet& delegates)
{

}

// static
CefRefPtr<CefPrintHandler> ClientAppBrowser::CreatePrintHandler()
{
#if defined(OS_LINUX)
  return new ClientPrintHandlerGtk();
#else
  return NULL;
#endif
}


}  // namespace browser
