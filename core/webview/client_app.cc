#include "client_app.h"

#include "include/cef_command_line.h"

namespace browser
{

namespace {
const char kProcessType[] = "type";
const char kRendererProcess[] = "renderer";
}  // namespace


ClientApp::ClientApp() {
}

// static
ClientApp::ProcessType ClientApp::GetProcessType(CefRefPtr<CefCommandLine> command_line) 
{
  // The command-line flag won't be specified for the browser process.
  if (!command_line->HasSwitch(kProcessType))
    return BrowserProcess;

  const std::string& process_type = command_line->GetSwitchValue(kProcessType);
  if (process_type == kRendererProcess)
    return RendererProcess;

  return OtherProcess;
}

void ClientApp::OnRegisterCustomSchemes(
	CefRawPtr<CefSchemeRegistrar> registrar) {
  RegisterCustomSchemes(registrar, cookieable_schemes_);
}


// static
void ClientApp::RegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar,std::vector<CefString>& cookiable_schemes)
{
   // scheme_test::RegisterCustomSchemes(registrar, cookiable_schemes);
}

}  // namespace browser
