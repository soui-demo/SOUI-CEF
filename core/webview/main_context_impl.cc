#include "stdafx.h"
#include "main_context_impl.h"
#include "include/cef_parser.h"
#include <direct.h>
#include <shlobj.h>
#include "client_app_browser.h"
#include "resource_util.h"
#include "customer_scheme.h"
#include "include/wrapper/cef_resource_manager.h"
#include "include/wrapper/cef_stream_resource_handler.h"

namespace browser {

namespace {
const char kDefaultUrl[] = "http://www.duowan.com";
const char kTestOrigin[] = "http://tests/";


// Add a file extension to |url| if none is currently specified.
std::string RequestUrlFilter(const std::string& url) {
	if (url.find(kTestOrigin) != 0U) {
		// Don't filter anything outside of the test origin.
		return url;
	}

	// Identify where the query or fragment component, if any, begins.
	size_t suffix_pos = url.find('?');
	if (suffix_pos == std::string::npos)
		suffix_pos = url.find('#');

	std::string url_base, url_suffix;
	if (suffix_pos == std::string::npos) {
		url_base = url;
	} else {
		url_base = url.substr(0, suffix_pos);
		url_suffix = url.substr(suffix_pos);
	}

	// Identify the last path component.
	size_t path_pos = url_base.rfind('/');
	if (path_pos == std::string::npos)
		return url;

	const std::string& path_component = url_base.substr(path_pos);

	// Identify if a file extension is currently specified.
	size_t ext_pos = path_component.rfind(".");
	if (ext_pos != std::string::npos)
		return url;

	// Rebuild the URL with a file extension.
	return url_base + ".html" + url_suffix;
}
}  // namespace

MainContextImpl::MainContextImpl(bool terminate_when_all_windows_closed)
	: terminate_when_all_windows_closed_(terminate_when_all_windows_closed),
	initialized_(false),
	shutdown_(false) {
	// Set the main URL.
	main_url_ = kDefaultUrl;
}

MainContextImpl::~MainContextImpl() {
	// The context must either not have been initialized, or it must have also
	// been shut down.
	DCHECK(!initialized_ || shutdown_);
}

// Returns dev tools client
IBrowserClient *MainContextImpl::GetDevToolsClient() {
	return dev_tools_client_;
}

void MainContextImpl::SetDevToolsClient(IBrowserClient *pBrowserClient) {
	dev_tools_client_ = pBrowserClient;
}

std::string MainContextImpl::GetConsoleLogPath() {
	return GetAppWorkingDirectory() + "console.log";
}

std::string MainContextImpl::GetMainURL() {
	return main_url_;
}

// Provider that dumps the request contents.
class RequestDumpResourceProvider : public CefResourceManager::Provider {
public:
	explicit RequestDumpResourceProvider(const std::string& url)
		: url_(url) {
		DCHECK(!url.empty());
	}

	bool OnRequest(scoped_refptr<CefResourceManager::Request> request) OVERRIDE {
		CEF_REQUIRE_IO_THREAD();

		const std::string& url = request->url();
		if (url != url_) {
			// Not handled by this provider.
			return false;
		}

		const std::string& dump = browser::DumpRequestContents(request->request());
		std::string str = "<html><body bgcolor=\"white\"><pre>" + dump +
			"</pre></body></html>";
		CefRefPtr<CefStreamReader> stream =
			CefStreamReader::CreateForData(
				static_cast<void*>(const_cast<char*>(str.c_str())),
				str.size());
		DCHECK(stream.get());
		request->Continue(new CefStreamResourceHandler("text/html", stream));
		return true;
	}

private:
	std::string url_;

	DISALLOW_COPY_AND_ASSIGN(RequestDumpResourceProvider);
};

void MainContext::DoMessageLoopWork() {
	CefDoMessageLoopWork();
}

CefRefPtr<CefApp> MainContext::InitCef3(const CefMainArgs& main_args, void* windows_sandbox_info) {
	// Enable High-DPI support on Windows 7 or newer.
	CefEnableHighDPISupport();

	// Parse command-line arguments.
	CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
	command_line->InitFromString(::GetCommandLineW());

	// Create a ClientApp of the correct type.
	CefRefPtr<CefApp> app;
	ClientApp::ProcessType process_type = ClientApp::GetProcessType(command_line);
	if (process_type == ClientApp::BrowserProcess)
		app = new ClientAppBrowser();

	return app;
}

bool MainContextImpl::Initialize(const CefMainArgs& main_args, CefRefPtr<CefApp> app, void* windows_sandbox_info) {
	DCHECK(thread_checker_.CalledOnValidThread());
	DCHECK(!initialized_);
	DCHECK(!shutdown_);


	CefSettings settings;


#if !defined(CEF_USE_SANDBOX)
	settings.no_sandbox = true;
#endif

	settings.windowless_rendering_enabled = true;
	//settings.multi_threaded_message_loop = true;
	CefString(&settings.cache_path) = SApplication::getSingleton().GetAppDir();
	//CefString(&settings.resources_dir_path) = SApplication::getSingleton().GetAppDir() + L"\\cef\\";
	//CefString(&settings.locales_dir_path) = SApplication::getSingleton().GetAppDir() + L"\\cef\\locales";
	//
	// CEF Initiaized
	//
	if (!CefInitialize(main_args, settings, app, windows_sandbox_info))
		return false;

	// Register scheme handlers.
	browser::scheme::RegisterSchemeHandlers();

	initialized_ = true;

	return true;
}

void MainContextImpl::Shutdown() {
	DCHECK(thread_checker_.CalledOnValidThread());
	DCHECK(initialized_);
	DCHECK(!shutdown_);

	CefShutdown();

	shutdown_ = true;
}


std::string MainContextImpl::GetDownloadPath(const std::string& file_name) {
	TCHAR szFolderPath[MAX_PATH];
	std::string path;

	// Save the file in the user's "My Documents" folder.
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
								  NULL, 0, szFolderPath))) {
		path = CefString(szFolderPath);
		path += "\\" + file_name;
	}

	return path;
}

std::string MainContextImpl::GetAppWorkingDirectory() {
	char szWorkingDir[MAX_PATH + 1];
	if (_getcwd(szWorkingDir, MAX_PATH) == NULL) {
		szWorkingDir[0] = 0;
	} else {
		// Add trailing path separator.
		size_t len = strlen(szWorkingDir);
		szWorkingDir[len] = '\\';
		szWorkingDir[len + 1] = 0;
	}
	return szWorkingDir;
}

//return the opened browser count
int MainContextImpl::GetOpenedBrowserCount() {
	return browser_opened_;
}

int  MainContextImpl::AddOpenedBrowserCount() {
	return ++browser_opened_;
}

int  MainContextImpl::DelOpenedBrowserCount() {
	return --browser_opened_;
}

}  // namespace browser
