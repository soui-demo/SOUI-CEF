#include "webview_handler.h"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_parser.h"
#include <shlobj.h>
#include "util_win.h"
#include "main_context.h" 
#include "resource_util.h" 
//#include "temp_window_win.h"
 
namespace browser {

#if defined(OS_WIN)
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif



// Custom menu command Ids.
enum browser_menu_ids {
	BROWSER_ID_SHOW_DEVTOOLS = MENU_ID_USER_FIRST,
	BROWSER_ID_CLOSE_DEVTOOLS,
	BROWSER_ID_INSPECT_ELEMENT,
};


std::string GetErrorString(cef_errorcode_t code) {
	// Case condition that returns |code| as a string.
#define CASE(code) case code: return #code

	switch (code) {
		CASE(ERR_NONE);
		CASE(ERR_FAILED);
		CASE(ERR_ABORTED);
		CASE(ERR_INVALID_ARGUMENT);
		CASE(ERR_INVALID_HANDLE);
		CASE(ERR_FILE_NOT_FOUND);
		CASE(ERR_TIMED_OUT);
		CASE(ERR_FILE_TOO_BIG);
		CASE(ERR_UNEXPECTED);
		CASE(ERR_ACCESS_DENIED);
		CASE(ERR_NOT_IMPLEMENTED);
		CASE(ERR_CONNECTION_CLOSED);
		CASE(ERR_CONNECTION_RESET);
		CASE(ERR_CONNECTION_REFUSED);
		CASE(ERR_CONNECTION_ABORTED);
		CASE(ERR_CONNECTION_FAILED);
		CASE(ERR_NAME_NOT_RESOLVED);
		CASE(ERR_INTERNET_DISCONNECTED);
		CASE(ERR_SSL_PROTOCOL_ERROR);
		CASE(ERR_ADDRESS_INVALID);
		CASE(ERR_ADDRESS_UNREACHABLE);
		CASE(ERR_SSL_CLIENT_AUTH_CERT_NEEDED);
		CASE(ERR_TUNNEL_CONNECTION_FAILED);
		CASE(ERR_NO_SSL_VERSIONS_ENABLED);
		CASE(ERR_SSL_VERSION_OR_CIPHER_MISMATCH);
		CASE(ERR_SSL_RENEGOTIATION_REQUESTED);
		CASE(ERR_CERT_COMMON_NAME_INVALID);
		CASE(ERR_CERT_DATE_INVALID);
		CASE(ERR_CERT_AUTHORITY_INVALID);
		CASE(ERR_CERT_CONTAINS_ERRORS);
		CASE(ERR_CERT_NO_REVOCATION_MECHANISM);
		CASE(ERR_CERT_UNABLE_TO_CHECK_REVOCATION);
		CASE(ERR_CERT_REVOKED);
		CASE(ERR_CERT_INVALID);
		CASE(ERR_CERT_END);
		CASE(ERR_INVALID_URL);
		CASE(ERR_DISALLOWED_URL_SCHEME);
		CASE(ERR_UNKNOWN_URL_SCHEME);
		CASE(ERR_TOO_MANY_REDIRECTS);
		CASE(ERR_UNSAFE_REDIRECT);
		CASE(ERR_UNSAFE_PORT);
		CASE(ERR_INVALID_RESPONSE);
		CASE(ERR_INVALID_CHUNKED_ENCODING);
		CASE(ERR_METHOD_NOT_SUPPORTED);
		CASE(ERR_UNEXPECTED_PROXY_AUTH);
		CASE(ERR_EMPTY_RESPONSE);
		CASE(ERR_RESPONSE_HEADERS_TOO_BIG);
		CASE(ERR_CACHE_MISS);
		CASE(ERR_INSECURE_RESPONSE);
	default:
		return "UNKNOWN";
	}
}

std::string GetDataURI(const std::string& data,
					   const std::string& mime_type) {
	return "data:" + mime_type + ";base64," +
		CefURIEncode(CefBase64Encode(data.data(), data.size()), false).ToString();
}

// Load a data: URI containing the error message.
void LoadErrorPage(CefRefPtr<CefFrame> frame,
				   const std::string& failed_url,
				   cef_errorcode_t error_code,
				   const std::string& other_info) {
	std::stringstream ss;
	ss << "<html><head><title>Page failed to load</title></head>"
		"<body bgcolor=\"white\">"
		"<h3>Page failed to load.</h3>"
		"URL: <a href=\"" << failed_url << "\">" << failed_url << "</a>"
		"<br/>Error: " << GetErrorString(error_code) <<
		" (" << error_code << ")";

	if (!other_info.empty())
		ss << "<br/>" << other_info;

	ss << "</body></html>";
	frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}


SWebViewHandler::SWebViewHandler() : m_posr(NULL)
, m_bPageLoaded(FALSE)
, m_pMsgHandler(NULL)
, first_console_message_(true)
, console_log_file_(MainContext::Get()->GetConsoleLogPath()) {
	resource_manager_ = new CefResourceManager();
}

SWebViewHandler::~SWebViewHandler() {
}

void SWebViewHandler::CloseDevTools(CefRefPtr<CefBrowser> browser) {
	browser->GetHost()->CloseDevTools();
}

bool SWebViewHandler::CreatePopupWindow(
	CefRefPtr<CefBrowser> browser,
	bool is_devtools,
	const CefPopupFeatures& popupFeatures,
	CefWindowInfo& windowInfo,
	CefRefPtr<CefClient>& client,
	CefBrowserSettings& settings) {
	// Note: This method will be called on multiple threads.

	// The popup browser will be parented to a new native window.
	// Don't show URL bar and navigation buttons on DevTools windows.
	//MainContext::Get()->CreateBrowserPopup(is_osr(), popupFeatures, windowInfo, client, settings);

	return false;
}


// ------------------------------------------------------------------------------
// 
// cef handler callbacks
//
// ------------------------------------------------------------------------------

bool SWebViewHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser,
									   CefRect& rect) {
	CEF_REQUIRE_UI_THREAD();

	if (m_posr) {
		return m_posr->GetRootScreenRect(browser, rect);
	}

	return false;
}

bool SWebViewHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
	CEF_REQUIRE_UI_THREAD();

	if (m_posr) {
		return m_posr->GetViewRect(browser, rect);
	}

	return false;
}

bool SWebViewHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser,
									int viewX,
									int viewY,
									int& screenX,
									int& screenY) {
	CEF_REQUIRE_UI_THREAD();

	if (m_posr) {
		return m_posr->GetScreenPoint(browser, viewX, viewY, screenX, screenY);
	}

	return false;
}

bool SWebViewHandler::GetScreenInfo(CefRefPtr<CefBrowser> browser,
								   CefScreenInfo& screen_info) {
	CEF_REQUIRE_UI_THREAD();

	if (m_posr) {
		return m_posr->GetScreenInfo(browser, screen_info);
	}

	return false;
}

void SWebViewHandler::OnPopupShow(CefRefPtr<CefBrowser> browser,
								 bool show) {
	CEF_REQUIRE_UI_THREAD();

	if (m_posr) {
		return m_posr->OnPopupShow(browser, show);
	}
}

void SWebViewHandler::OnPopupSize(CefRefPtr<CefBrowser> browser,
								 const CefRect& rect) {
	CEF_REQUIRE_UI_THREAD();

	if (m_posr) {
		return m_posr->OnPopupSize(browser, rect);
	}
}

void SWebViewHandler::OnPaint(CefRefPtr<CefBrowser> browser,
							 PaintElementType type,
							 const RectList & dirtyRects,
							 const void* buffer, int width, int height) {
	CEF_REQUIRE_UI_THREAD();

	if (m_posr) {
		m_posr->OnPaint(browser, type, dirtyRects, buffer, width, height);
	}
}

void SWebViewHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,
									CefCursorHandle cursor,
									CursorType type,
									const CefCursorInfo& custom_cursor_info) {
	CEF_REQUIRE_UI_THREAD();

	if (m_posr) {
		m_posr->OnCursorChange(browser, cursor, type, custom_cursor_info);
	}
}

bool SWebViewHandler::StartDragging(
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefDragData> drag_data,
	CefRenderHandler::DragOperationsMask allowed_ops,
	int x, int y) {
	CEF_REQUIRE_UI_THREAD();
	return false;
}

void SWebViewHandler::UpdateDragCursor(
	CefRefPtr<CefBrowser> browser,
	CefRenderHandler::DragOperation operation) {
	CEF_REQUIRE_UI_THREAD();

	if (m_posr) {
		m_posr->UpdateDragCursor(browser, operation);
	}
}

void SWebViewHandler::OnImeCompositionRangeChanged(
	CefRefPtr<CefBrowser> browser,
	const CefRange& selection_range,
	const CefRenderHandler::RectList& character_bounds) {
	CEF_REQUIRE_UI_THREAD();
	if (m_posr) {
		m_posr->OnImeCompositionRangeChanged(browser, selection_range, character_bounds);
	}
}

bool SWebViewHandler::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
								   const CefKeyEvent& event,
								   CefEventHandle os_event,
								   bool* is_keyboard_shortcut) {
	if (!m_bPageLoaded) {
		return true;
	}

	CefWindowHandle hWnd = browser->GetHost()->GetWindowHandle();

	//
	// 退出
	if (event.character == VK_ESCAPE) {
		::PostMessage(::GetParent(hWnd), WM_KEYDOWN, event.character, 0);
		return true;
	}

	//
	// 调试快捷键
	if (IsKeyDown(VK_SHIFT)) {
		STRACE(L"char:%d", event.character);
		if (event.character == VK_F12) {
			OpenDevTools();
		} else if (event.character == VK_F11) {
			browser->GetMainFrame()->ViewSource();
		}
	}

	return false;
}

bool SWebViewHandler::DoClose(CefRefPtr<CefBrowser> browser) {
	return false;
}

void SWebViewHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();
	m_refBrowser = browser;

	browser::MainContext::Get()->AddOpenedBrowserCount();
	if (m_posr) {
		m_posr->OnAfterCreated(browser);
	}
}

void  SWebViewHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {

	if (m_refBrowser.get() &&
		m_refBrowser->GetIdentifier() == browser->GetIdentifier()) {
		// !!! IMPORTANT !!!
		m_refBrowser = NULL;
		STRACE(L"main browser closed");
	}

	int n = browser::MainContext::Get()->DelOpenedBrowserCount();
	//STRACE(L"browser left:%d", n);
	if (n == 0) {
		STRACE(L"quit message loop");
		//CefQuitMessageLoop();
	}
}


// ------------------------------------------------------------------------------
// 
// browser methods for user & internal methods
//
// ------------------------------------------------------------------------------

void SWebViewHandler::OpenDevTools() {
	if (!m_refBrowser) {
		return;
	}

	CefWindowInfo windowInfo;
	CefBrowserSettings settings;

	windowInfo.SetAsPopup(m_refBrowser->GetHost()->GetWindowHandle(), "DevTools");
	m_refBrowser->GetHost()->ShowDevTools(windowInfo, this, settings, CefPoint());
}

bool SWebViewHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
								   CefRefPtr<CefFrame> frame,
								   const CefString& target_url,
								   const CefString& target_frame_name,
								   CefLifeSpanHandler::WindowOpenDisposition target_disposition,
								   bool user_gesture,
								   const CefPopupFeatures& popupFeatures,
								   CefWindowInfo& windowInfo,
								   CefRefPtr<CefClient>& client,
								   CefBrowserSettings& settings,
								   bool* no_javascript_access) {
	if (m_posr) {
		m_posr->OnBeforePopup(browser, frame, target_url, target_frame_name, target_disposition, user_gesture,
							  popupFeatures, windowInfo, client, settings, no_javascript_access);
	}
	return true;//不允许弹窗口，由应用自己决定弹不弹
}


BOOL SWebViewHandler::Open(HWND hParent, SOUI::CRect rcView) {
	CefWindowInfo info;
	info.SetAsWindowless(hParent, true);

	return !!CefBrowserHost::CreateBrowser(info, this, "", CefBrowserSettings(), nullptr);
}

void SWebViewHandler::Close() {
	if (m_refBrowser)
		m_refBrowser->GetHost()->CloseBrowser(true);
}

void SWebViewHandler::CloseAllBrowsers(bool force_close) {
	if (!CefCurrentlyOn(TID_UI)) {
		// Execute on the UI thread.
		CefPostTask(TID_UI, base::Bind(&SWebViewHandler::CloseAllBrowsers, this, force_close));

		return;
	}

	if (m_refBrowser.get()) {
		// Request that the main browser close.
		m_refBrowser->GetHost()->CloseBrowser(force_close);
	}
}

void SWebViewHandler::SetRender(OsrDelegate * pHost) {
	m_posr = pHost;
}

CefRefPtr<CefBrowser> SWebViewHandler::GetBrowser() {
	return m_refBrowser;
}

void SWebViewHandler::RegisterMessageHandler(MessageHandler * handler) {
	m_pMsgHandler = handler;
}

void SWebViewHandler::UnRegisterMessgeHandler(MessageHandler * handler) {
	m_pMsgHandler = NULL;
}


void SWebViewHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
								   const CefString& title) {
}


bool SWebViewHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
									  const CefString& message,
									  const CefString& source,
									  int line) {
	CEF_REQUIRE_UI_THREAD();

	FILE* file = fopen(console_log_file_.c_str(), "a");
	if (file) {
		std::stringstream ss;
		ss << "Message: " << message.ToString() << NEWLINE <<
			"Source: " << source.ToString() << NEWLINE <<
			"Line: " << line << NEWLINE <<
			"-----------------------" << NEWLINE;
		fputs(ss.str().c_str(), file);
		fclose(file);

		if (first_console_message_) {
			//test_runner::Alert( browser, "Console messages written to \"" + console_log_file_ + "\"");
			first_console_message_ = false;
		}
	}

	return false;
}

void SWebViewHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
									 CefRefPtr<CefFrame> frame,
									 const CefString& url) {
	if (m_posr) {
		m_posr->OnAddressChange(browser, frame, url);
	}
}

void SWebViewHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
										 CefRefPtr<CefFrame> frame,
										 CefRefPtr<CefContextMenuParams> params,
										 CefRefPtr<CefMenuModel> model) {
	model->Clear();//没有菜单
}

bool SWebViewHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
									CefRefPtr<CefFrame> frame,
									CefRefPtr<CefRequest> request,
									bool is_redirect) {
	CEF_REQUIRE_UI_THREAD();

	//message_router_->OnBeforeBrowse(browser, frame);
	return false;
}


bool SWebViewHandler::OnOpenURLFromTab(
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	const CefString& target_url,
	CefRequestHandler::WindowOpenDisposition target_disposition,
	bool user_gesture) {
	if (target_disposition == WOD_NEW_BACKGROUND_TAB ||
		target_disposition == WOD_NEW_FOREGROUND_TAB) {
		// Handle middle-click and ctrl + left-click by opening the URL in a new
		// browser window.
		return true;
	}

	// Open the URL in the current browser window.
	return false;
}


cef_return_value_t SWebViewHandler::OnBeforeResourceLoad(
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefRequest> request,
	CefRefPtr<CefRequestCallback> callback) {
	CEF_REQUIRE_IO_THREAD();

	return resource_manager_->OnBeforeResourceLoad(browser, frame, request, callback);
}


CefRefPtr<CefResourceHandler> SWebViewHandler::GetResourceHandler(
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefRequest> request) {
	CEF_REQUIRE_IO_THREAD();

	return resource_manager_->GetResourceHandler(browser, frame, request);
}

CefRefPtr<CefResponseFilter> SWebViewHandler::GetResourceResponseFilter(
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefRequest> request,
	CefRefPtr<CefResponse> response) {
	CEF_REQUIRE_IO_THREAD();

	return NULL;
	//return test_runner::GetResourceResponseFilter(browser, frame, request, response);
}

bool SWebViewHandler::OnQuotaRequest(CefRefPtr<CefBrowser> browser,
									const CefString& origin_url,
									int64 new_size,
									CefRefPtr<CefRequestCallback> callback) {
	CEF_REQUIRE_IO_THREAD();

	static const int64 max_size = 1024 * 1024 * 20;  // 20mb.

	// Grant the quota request if the size is reasonable.
	callback->Continue(new_size <= max_size);
	return true;
}

void SWebViewHandler::OnProtocolExecution(CefRefPtr<CefBrowser> browser,
										 const CefString& url,
										 bool& allow_os_execution) {
	CEF_REQUIRE_UI_THREAD();

	std::string urlStr = url;

	// Allow OS execution of Spotify URIs.
	if (urlStr.find("spotify:") == 0)
		allow_os_execution = true;
}


bool SWebViewHandler::OnCertificateError(
	CefRefPtr<CefBrowser> browser,
	ErrorCode cert_error,
	const CefString& request_url,
	CefRefPtr<CefSSLInfo> ssl_info,
	CefRefPtr<CefRequestCallback> callback) {
	CEF_REQUIRE_UI_THREAD();
	CefRefPtr<CefX509Certificate> cert = ssl_info->GetX509Certificate();
	CefRefPtr<CefX509CertPrincipal> subject = cert->GetSubject();
	CefRefPtr<CefX509CertPrincipal> issuer = cert->GetIssuer();

	// Build a table showing certificate information. Various types of invalid
	// certificates can be tested using https://badssl.com/.
	std::stringstream ss;
	ss << "X.509 Certificate Information:"
		"<table border=1><tr><th>Field</th><th>Value</th></tr>" <<
		"<tr><td>Subject</td><td>" <<
		(subject.get() ? subject->GetDisplayName().ToString() : "&nbsp;") <<
		"</td></tr>"
		"<tr><td>Issuer</td><td>" <<
		(issuer.get() ? issuer->GetDisplayName().ToString() : "&nbsp;") <<
		"</td></tr>"
		"<tr><td>Serial #*</td><td>" <<
		browser::GetBinaryString(cert->GetSerialNumber()) << "</td></tr>"
		"<tr><td>Status</td><td>" <<
		browser::GetCertStatusString(ssl_info->GetCertStatus()) << "</td></tr>"
		"<tr><td>Valid Start</td><td>" <<
		browser::GetTimeString(cert->GetValidStart()) << "</td></tr>"
		"<tr><td>Valid Expiry</td><td>" <<
		browser::GetTimeString(cert->GetValidExpiry()) << "</td></tr>";

	CefX509Certificate::IssuerChainBinaryList der_chain_list;
	CefX509Certificate::IssuerChainBinaryList pem_chain_list;
	cert->GetDEREncodedIssuerChain(der_chain_list);
	cert->GetPEMEncodedIssuerChain(pem_chain_list);
	DCHECK_EQ(der_chain_list.size(), pem_chain_list.size());

	der_chain_list.insert(der_chain_list.begin(), cert->GetDEREncoded());
	pem_chain_list.insert(pem_chain_list.begin(), cert->GetPEMEncoded());

	for (size_t i = 0U; i < der_chain_list.size(); ++i) {
		ss << "<tr><td>DER Encoded*</td>"
			"<td style=\"max-width:800px;overflow:scroll;\">" <<
			browser::GetBinaryString(der_chain_list[i]) << "</td></tr>"
			"<tr><td>PEM Encoded*</td>"
			"<td style=\"max-width:800px;overflow:scroll;\">" <<
			browser::GetBinaryString(pem_chain_list[i]) << "</td></tr>";
	}

	ss << "</table> * Displayed value is base64 encoded.";

	// Load the error page.
	LoadErrorPage(browser->GetMainFrame(), request_url, cert_error, ss.str());

	return false;  // Cancel the request.
}


void SWebViewHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
											   TerminationStatus status) {
	CEF_REQUIRE_UI_THREAD();

	//message_router_->OnRenderProcessTerminated(browser);

	// Don't reload if there's no start URL, or if the crash URL was specified.
	if (browser::MainContext::Get()->GetMainURL().empty() || browser::MainContext::Get()->GetMainURL() == "chrome://crash")
		return;

	CefRefPtr<CefFrame> frame = browser->GetMainFrame();
	std::string url = frame->GetURL();

	// Don't reload if the termination occurred before any URL had successfully
	// loaded.
	if (url.empty())
		return;

	std::string start_url = browser::MainContext::Get()->GetMainURL();

	// Convert URLs to lowercase for easier comparison.
	std::transform(url.begin(), url.end(), url.begin(), tolower);
	std::transform(start_url.begin(), start_url.end(), start_url.begin(),
				   tolower);

	// Don't reload the URL that just resulted in termination.
	if (url.find(start_url) == 0)
		return;

	frame->LoadURL(browser::MainContext::Get()->GetMainURL());
}



} //namespace browser