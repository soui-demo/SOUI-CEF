#include "stdafx.h"
#include "geometry_util.h"
#include "webview.h"
#include "util_win.h"
#include "extend_events.h"
#include "main_context.h"
#include "osr_ime_handler_win.h"

namespace SOUI {

#define CHECK_CEF_BROWSER_HOST \
	if (first_show_) return; \
    if ( !webview_handler_ ) return; \
    CefRefPtr<CefBrowser> pb = webview_handler_->GetBrowser(); \
    if ( !pb.get() ) return;\
    CefRefPtr<CefBrowserHost> host = pb->GetHost(); \
    if (!host) return;

#define CHECK_CEF_BROWSER_HOST_EX \
    if ( !webview_handler_ ) return 0; \
    CefRefPtr<CefBrowser> pb = webview_handler_->GetBrowser(); \
    if ( !pb.get() ) return 0;\
    CefRefPtr<CefBrowserHost> host = pb->GetHost(); \
    if (!host) return 0

SWebView::SWebView(void)
	: m_bSkipCursor(FALSE)
	, m_bInternalPaint(FALSE)
	, m_bBkgndDirty(TRUE)
	, hidden_(false)
	, first_show_(true)
	, painting_popup_(false)
	, device_scale_factor_(browser::GetDeviceScaleFactor())
	, resize_popup_(false)
	, last_click_x_(0)
	, last_click_y_(0)
	, last_click_time_(0)
	, last_mouse_pos_()
	, current_mouse_pos_()
	, mouse_rotation_(false)
	, mouse_tracking_(false)
	, last_click_count_(0)
	, last_click_button_(MBT_LEFT)
	, last_mouse_down_on_view_(false)
	, draw_popup_(false) {

	m_evtSet.addEvent(EVENTID(EventWebViewNotify));
	m_evtSet.addEvent(EVENTID(EventURLChange));
	m_evtSet.addEvent(EVENTID(EventWebviewLoadEnd));

	m_bDrawFocusRect = FALSE;
	webview_handler_ = new browser::SWebViewHandler;
	webview_handler_->AddRef();

	m_bFocusable = TRUE;
}

SWebView::~SWebView(void) {
	webview_handler_->Release();
}

void SWebView::SetDevTools(bool devtools) {
	browser::MainContext::Get()->SetDevToolsClient(this);
}
// ------------------------------------------------------------------------------
//
// methods for user
//
// ------------------------------------------------------------------------------ 

BOOL SWebView::LoadURL(const SStringW& url) {
	if (!webview_handler_) {
		return FALSE;
	}

	CefRefPtr<CefBrowser> pb = webview_handler_->GetBrowser();
	if (pb.get()) {
		CefRefPtr<CefFrame> frame = pb->GetMainFrame();
		if (!frame)
			return FALSE;

		m_strUrl = url;
		frame->LoadURL((LPCWSTR) url);
	}
	return TRUE;
}

SStringW SWebView::GetTitle() {
	return m_strTitle;
}

SStringW SWebView::GetURL() {
	return m_strUrl;
}

void SWebView::ExecJavaScript(const SStringW& js) {
	if (!webview_handler_) {
		return;
	}

	CefRefPtr<CefBrowser> pb = webview_handler_->GetBrowser();
	if (pb.get()) {
		CefRefPtr<CefFrame> frame = pb->GetMainFrame();
		if (frame) {
			frame->ExecuteJavaScript((LPCWSTR) js, L"", 0);
		}
	}
}

BOOL SWebView::CanGoBack() {
	if (!webview_handler_) {
		return FALSE;
	}

	CefRefPtr<CefBrowser> pb = webview_handler_->GetBrowser();
	if (pb.get()) {
		return pb->CanGoBack();
	}

	return FALSE;
}

void SWebView::GoBack() {
	if (!webview_handler_) {
		return;
	}

	CefRefPtr<CefBrowser> pb = webview_handler_->GetBrowser();
	if (pb.get()) {
		return pb->GoBack();
	}
}

BOOL SWebView::CanGoForward() {
	if (!webview_handler_) {
		return FALSE;
	}

	CefRefPtr<CefBrowser> pb = webview_handler_->GetBrowser();
	if (pb.get()) {
		return pb->CanGoForward();
	}

	return FALSE;
}

void SWebView::GoForward() {
	if (!webview_handler_) {
		return;
	}

	CefRefPtr<CefBrowser> pb = webview_handler_->GetBrowser();
	if (pb.get()) {
		return pb->GoForward();
	}
}

BOOL SWebView::IsLoading() {
	if (!webview_handler_) {
		return FALSE;
	}

	CefRefPtr<CefBrowser> pb = webview_handler_->GetBrowser();
	if (pb.get()) {
		return pb->IsLoading();
	}

	return FALSE;
}

void SWebView::Reload() {
	if (!webview_handler_) {
		return;
	}

	CefRefPtr<CefBrowser> pb = webview_handler_->GetBrowser();
	if (pb.get()) {
		return pb->Reload();
	}
}

void SWebView::StopLoad() {
	if (!webview_handler_) {
		return;
	}

	CefRefPtr<CefBrowser> pb = webview_handler_->GetBrowser();
	if (pb.get()) {
		return pb->StopLoad();
	}
}

BOOL SWebView::Open() {
	CefRefPtr<CefBrowser> pb = webview_handler_->GetBrowser();
	if (pb.get()) {
		return TRUE;
	}

	if (!webview_handler_->Open(GetContainer()->GetHostHwnd(), GetClientRect())) {
		return FALSE;
	}

	webview_handler_->SetRender(this);
	webview_handler_->RegisterMessageHandler(this);
	return TRUE;
}

void SWebView::Close() {
	webview_handler_->CloseAllBrowsers(true);
	webview_handler_->SetRender(NULL);
	webview_handler_->UnRegisterMessgeHandler(this);
	m_bBkgndDirty = TRUE;
	m_pBitmapBuff = NULL;
}

void SWebView::GetWebViewSource(std::function<void(const CefString&)> &&callback) {
	class Visitor : public CefStringVisitor {
	public:
		explicit Visitor(std::function<void(const CefString&)> &&cb)
			: callback_(std::move(cb)) {
		}
		virtual void Visit(const CefString& strSource) OVERRIDE {
			callback_(strSource);
		}

	private:
		std::function<void(const CefString &)> callback_;
		IMPLEMENT_REFCOUNTING(Visitor);
	};
	webview_handler_->GetBrowser()->GetMainFrame()->GetSource(new Visitor(std::move(callback)));
}

void SWebView::UpdateBkgndRenderTarget() {
	// 更新背景RenderTarget
	CRect rcWnd = GetClientRect();
	if (!m_prtBackground) {
		GETRENDERFACTORY->CreateRenderTarget(&m_prtBackground, rcWnd.Width(), rcWnd.Height());
	} else {
		m_prtBackground->Resize(rcWnd.Size());
	}
	m_prtBackground->SetViewportOrg(-rcWnd.TopLeft());
	m_bBkgndDirty = TRUE;
}

int SWebView::OnCreate(LPVOID) {
	ime_handler_.reset(
		new browser::OsrImeHandlerWin(GetContainer()->GetHostHwnd()));
	Open();
	if (!m_strUrl.IsEmpty()) {
		LoadURL(m_strUrl);
	}

	return 0;
}

void SWebView::OnSize(UINT nType, CSize size) {
	STRACE(L"webview resized");

	SWindow::OnSize(nType, size);
	UpdateBkgndRenderTarget();

	if (ime_handler_) {//调整输入法窗口位置.办法有点笨
		CPoint &pt = GetWindowRect().TopLeft();
		ime_handler_->UpdateHostPosition(CefPoint(pt.x, pt.y));
	}

	if (!m_pBitmapBuff || (m_pBitmapBuff->Width() != (unsigned int) size.cx || m_pBitmapBuff->Height() != (unsigned int) size.cy)) {
		CHECK_CEF_BROWSER_HOST;
		host->WasResized();
	}

}

BOOL SWebView::OnAttrUrl(SStringW strValue, BOOL bLoading) {
	m_strUrl = strValue;

	if (!bLoading) {
		LoadURL(strValue);
	}

	return !bLoading;
}

void SWebView::OnDestroy() {
	STRACE(L"webview destroy");

	GetContainer()->RevokeDragDrop(GetSwnd());
	ime_handler_.reset();
	if (webview_handler_) {
		Close();
	}

	SWindow::OnDestroy();
}

void SWebView::OnPaint(IRenderTarget *pRT) {
	//STRACE(L"web view onpaint");
	//LOGI(__FILE__, GetCurrentThreadId());

	CRect rcWnd = GetClientRect();

	if (m_bBkgndDirty && m_prtBackground) {
		m_bBkgndDirty = FALSE;
		m_prtBackground->BitBlt(rcWnd, pRT, rcWnd.left, rcWnd.top);
	}

	SWindow::OnPaint(pRT);

	if (!first_show_ && webview_handler_) {
		CRect rcClipBox;
		pRT->GetClipBox(rcClipBox);
		rcClipBox.IntersectRect(rcClipBox, rcWnd);

		CHECK_CEF_BROWSER_HOST;

		// request cef to update buff bitmap
		m_bInternalPaint = TRUE;

		host->Invalidate(PET_VIEW);

		m_bInternalPaint = FALSE;
	}

	if (m_pBitmapBuff) {
		rcWnd.right = rcWnd.left + m_pBitmapBuff->Width();
		rcWnd.bottom = rcWnd.top + m_pBitmapBuff->Height();
		pRT->DrawBitmap(rcWnd, m_pBitmapBuff, 0, 0, 0xFF);
	}
}

LRESULT SWebView::OnCaptureChanged(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	SendCaptureLostEvent(uMsg, wParam, lParam);
	return 0;
}

LRESULT SWebView::OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	SendMouseEvent(uMsg, wParam, lParam);
	return 0;
}

LRESULT SWebView::OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	SendKeyEvent(uMsg, wParam, lParam);
	return 0;
}

void SWebView::OnSetFocus(SWND wndOld) {
	CHECK_CEF_BROWSER_HOST;

	STRACE(L"set browser focus");
	host->SendFocusEvent(true);
}

void SWebView::OnKillFocus(SWND wndFocus) {
	CHECK_CEF_BROWSER_HOST;

	STRACE(L"kill browser focus");

	host->SendFocusEvent(false);
}

void SWebView::OnShowWindow(BOOL bShow, UINT nStatus) {
	SWindow::OnShowWindow(bShow, nStatus);

	if (first_show_ && IsVisible(TRUE)) {
		first_show_ = false;
	}

	CHECK_CEF_BROWSER_HOST;
	host->WasHidden(!bShow);

	if (bShow) {
		SetFocus();
		host->SendFocusEvent(true);
	}
}

BOOL SWebView::OnSetCursor(const CPoint &pt) {
	return TRUE;
}

LRESULT SWebView::OnIMESetContext(UINT message, WPARAM wParam, LPARAM lParam) {
	lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
	::DefWindowProc(GetContainer()->GetHostHwnd(), message, wParam, lParam);

	// Create Caret Window if required
	if (ime_handler_) {
		CPoint &pt = GetWindowRect().TopLeft();
		ime_handler_->CreateImeWindow();
		ime_handler_->MoveImeWindow();
	}

	return 0;
}

LRESULT SWebView::OnIMEStartComposition(UINT message, WPARAM wParam, LPARAM lParam) {
	if (ime_handler_) {
		CPoint &pt = GetWindowRect().TopLeft();
		ime_handler_->CreateImeWindow();
		ime_handler_->MoveImeWindow();
		ime_handler_->ResetComposition();
	}
	return 0;
}

LRESULT SWebView::OnIMEComposition(UINT message, WPARAM wParam, LPARAM lParam) {
	if (webview_handler_->GetBrowser() && ime_handler_) {
		CefRefPtr<CefBrowser> browser = webview_handler_->GetBrowser();
		CefString cTextStr;
		if (ime_handler_->GetResult(lParam, cTextStr)) {
			// Send the text to the browser. The |replacement_range| and
			// |relative_cursor_pos| params are not used on Windows, so provide
			// default invalid values.
			browser->GetHost()->ImeCommitText(cTextStr,
											  CefRange(UINT32_MAX, UINT32_MAX), 0);
			ime_handler_->ResetComposition();
			// Continue reading the composition string - Japanese IMEs send both
			// GCS_RESULTSTR and GCS_COMPSTR.
		}

		std::vector<CefCompositionUnderline> underlines;
		int composition_start = 0;

		if (ime_handler_->GetComposition(lParam, cTextStr, underlines,
										 composition_start)) {
			// Send the composition string to the browser. The |replacement_range|
			// param is not used on Windows, so provide a default invalid value.
			browser->GetHost()->ImeSetComposition(
				cTextStr, underlines, CefRange(UINT32_MAX, UINT32_MAX),
				CefRange(composition_start,
						 static_cast<int>(composition_start + cTextStr.length())));

			// Update the Candidate Window position. The cursor is at the end so
			// subtract 1. This is safe because IMM32 does not support non-zero-width
			// in a composition. Also,  negative values are safely ignored in
			// MoveImeWindow
			ime_handler_->UpdateCaretPosition(composition_start - 1);
		} else {
			OnIMECancelCompositionEvent(message, wParam, lParam);
		}
	}
	return 0;
}

LRESULT SWebView::OnIMECancelCompositionEvent(UINT message, WPARAM wParam, LPARAM lParam) {
	CefRefPtr<CefBrowser> browser = webview_handler_->GetBrowser();
	if (browser && ime_handler_) {
		browser->GetHost()->ImeCancelComposition();
		ime_handler_->ResetComposition();
		ime_handler_->DestroyImeWindow();
	}
	return 0;
}

// ------------------------------------------------------------------------------
//
// callback helper methods
//
// ------------------------------------------------------------------------------
//


void SWebView::SendFocusEvent(bool focus) {
	CHECK_CEF_BROWSER_HOST;

	STRACE(L"sent browser focus:%d", focus);
	host->SendFocusEvent(focus);
}

void SWebView::SendMouseEvent(UINT message, WPARAM wParam, LPARAM lParam) {
	CHECK_CEF_BROWSER_HOST;

	LONG currentTime = 0;
	bool cancelPreviousClick = false;

	if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
		message == WM_MBUTTONDOWN || message == WM_MOUSEMOVE ||
		message == WM_MOUSELEAVE) {
		currentTime = GetMessageTime();

		CRect rcView = GetClientRect();
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

		int x = pt.x - rcView.left;
		int y = pt.y - rcView.top;

		cancelPreviousClick =
			(abs(last_click_x_ - x) > (GetSystemMetrics(SM_CXDOUBLECLK) / 2))
			|| (abs(last_click_y_ - y) > (GetSystemMetrics(SM_CYDOUBLECLK) / 2))
			|| ((currentTime - last_click_time_) > GetDoubleClickTime());
		if (cancelPreviousClick &&
			(message == WM_MOUSEMOVE || message == WM_MOUSELEAVE)) {
			last_click_count_ = 0;
			last_click_x_ = 0;
			last_click_y_ = 0;
			last_click_time_ = 0;
		}
	}

	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	{
		SetCapture();
		SetFocus();
		CRect rcView = GetClientRect();
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

		int x = pt.x - rcView.left;
		int y = pt.y - rcView.top;

		if (wParam & MK_SHIFT) {
			// Start rotation effect.
			last_mouse_pos_.x = current_mouse_pos_.x = x;
			last_mouse_pos_.y = current_mouse_pos_.y = y;
			mouse_rotation_ = true;
		} else {
			CefBrowserHost::MouseButtonType btnType =
				(message == WM_LBUTTONDOWN ? MBT_LEFT : (
					message == WM_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));
			if (!cancelPreviousClick && (btnType == last_click_button_)) {
				++last_click_count_;
			} else {
				last_click_count_ = 1;
				last_click_x_ = x;
				last_click_y_ = y;
			}
			last_click_time_ = currentTime;
			last_click_button_ = btnType;

			if (host) {
				CefMouseEvent mouse_event;
				mouse_event.x = x;
				mouse_event.y = y;
				last_mouse_down_on_view_ = !IsOverPopupWidget(x, y);
				ApplyPopupOffset(mouse_event.x, mouse_event.y);
				browser::DeviceToLogical(mouse_event, device_scale_factor_);
				mouse_event.modifiers = browser::GetCefMouseModifiers(wParam);
				host->SendMouseClickEvent(mouse_event, btnType, false,
										  last_click_count_);
			}
		}
	} break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		if (::GetCapture() == GetContainer()->GetHostHwnd())
			ReleaseCapture();
		if (mouse_rotation_) {
			// End rotation effect.
			mouse_rotation_ = false;
			//renderer_.SetSpin(0, 0);
			Invalidate();
		} else {
			CRect rcView = GetClientRect();
			CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			int x = pt.x - rcView.left;
			int y = pt.y - rcView.top;

			CefBrowserHost::MouseButtonType btnType =
				(message == WM_LBUTTONUP ? MBT_LEFT : (
					message == WM_RBUTTONUP ? MBT_RIGHT : MBT_MIDDLE));
			if (host) {
				CefMouseEvent mouse_event;
				mouse_event.x = x;
				mouse_event.y = y;
				if (last_mouse_down_on_view_ &&
					IsOverPopupWidget(x, y) &&
					(GetPopupXOffset() || GetPopupYOffset())) {
					break;
				}
				ApplyPopupOffset(mouse_event.x, mouse_event.y);
				browser::DeviceToLogical(mouse_event, device_scale_factor_);
				mouse_event.modifiers = browser::GetCefMouseModifiers(wParam);
				host->SendMouseClickEvent(mouse_event, btnType, true,
										  last_click_count_);
			}
		}
		break;

	case WM_MOUSEMOVE:
	{
		CRect rcView = GetClientRect();
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

		int x = pt.x - rcView.left;
		int y = pt.y - rcView.top;

		bool leave = !rcView.PtInRect(pt) && !IsCaptured();

		if (leave)
			m_bSkipCursor = TRUE;
		else
			m_bSkipCursor = FALSE;

		if (mouse_rotation_) {
			// Apply rotation effect.
			current_mouse_pos_.x = x;
			current_mouse_pos_.y = y;
			//renderer_.IncrementSpin(
			//	current_mouse_pos_.x - last_mouse_pos_.x,
			//	current_mouse_pos_.y - last_mouse_pos_.y);
			last_mouse_pos_.x = current_mouse_pos_.x;
			last_mouse_pos_.y = current_mouse_pos_.y;
			Invalidate();
		} else {
			if (!mouse_tracking_) {
				// Start tracking mouse leave. Required for the WM_MOUSELEAVE event to
				// be generated.
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = GetContainer()->GetHostHwnd();
				TrackMouseEvent(&tme);
				mouse_tracking_ = true;
			}

			if (host) {
				CefMouseEvent mouse_event;
				mouse_event.x = x;
				mouse_event.y = y;
				ApplyPopupOffset(mouse_event.x, mouse_event.y);
				browser::DeviceToLogical(mouse_event, device_scale_factor_);
				mouse_event.modifiers = browser::GetCefMouseModifiers(wParam);
				host->SendMouseMoveEvent(mouse_event, false);
			}
		}
		break;
	}

	case WM_MOUSELEAVE:
	{
		if (mouse_tracking_) {
			// Stop tracking mouse leave.
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE & TME_CANCEL;
			tme.hwndTrack = GetContainer()->GetHostHwnd();
			TrackMouseEvent(&tme);
			mouse_tracking_ = false;
		}

		if (host) {
			// Determine the cursor position in screen coordinates.
			POINT p;
			::GetCursorPos(&p);
			::ScreenToClient(GetContainer()->GetHostHwnd(), &p);

			CefMouseEvent mouse_event;
			mouse_event.x = p.x;
			mouse_event.y = p.y;
			browser::DeviceToLogical(mouse_event, device_scale_factor_);
			mouse_event.modifiers = browser::GetCefMouseModifiers(wParam);
			host->SendMouseMoveEvent(mouse_event, true);
		}
	} break;

	case WM_MOUSEWHEEL:
		if (host) {
			CRect rcView = GetClientRect();
			CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			::ClientToScreen(GetContainer()->GetHostHwnd(), &pt);

			POINT screen_point = {pt.x - rcView.left, pt.y - rcView.top};

			HWND scrolled_wnd = ::WindowFromPoint(screen_point);
			if (scrolled_wnd != GetContainer()->GetHostHwnd())
				break;

			::ScreenToClient(GetContainer()->GetHostHwnd(), &screen_point);
			int delta = GET_WHEEL_DELTA_WPARAM(wParam);

			CefMouseEvent mouse_event;
			mouse_event.x = screen_point.x;
			mouse_event.y = screen_point.y;
			ApplyPopupOffset(mouse_event.x, mouse_event.y);
			browser::DeviceToLogical(mouse_event, device_scale_factor_);
			mouse_event.modifiers = browser::GetCefMouseModifiers(wParam);
			host->SendMouseWheelEvent(mouse_event,
									  browser::IsKeyDown(VK_SHIFT) ? delta : 0,
									  !browser::IsKeyDown(VK_SHIFT) ? delta : 0);
		}
		break;
	}

}

void SWebView::SendKeyEvent(UINT uMsg, WPARAM wp, LPARAM lp) {
	CHECK_CEF_BROWSER_HOST;

	CefKeyEvent event;
	event.windows_key_code = wp;
	event.native_key_code = lp;
	event.is_system_key = uMsg == WM_SYSCHAR || uMsg == WM_SYSKEYDOWN ||
		uMsg == WM_SYSKEYUP;

	if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
		event.type = KEYEVENT_RAWKEYDOWN;
	else if (uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP)
		event.type = KEYEVENT_KEYUP;
	else
		event.type = KEYEVENT_CHAR;
	event.modifiers = browser::GetCefKeyboardModifiers(wp, lp);

	host->SendKeyEvent(event);
}

void SWebView::SendCaptureLostEvent(UINT uMsg, WPARAM wp, LPARAM lp) {
	if (mouse_rotation_)
		return;

	CHECK_CEF_BROWSER_HOST;

	STRACE(L"browser send capture losted");
	host->SendCaptureLostEvent();
}

void  SWebView::Draw(int x, int y, IBitmap * pbuff) {
	if (m_bInternalPaint || !m_prtBackground || m_bBkgndDirty) {
		STRACE(L"on ready...");
		return;
	}


	CRect rcWnd = GetClientRect();
	rcWnd.left += x;
	rcWnd.top += y;

	if ((unsigned int) rcWnd.Width() != pbuff->Width() || (unsigned int) rcWnd.Height() != pbuff->Height()) {
		rcWnd.right = rcWnd.left + pbuff->Width();
		rcWnd.bottom = rcWnd.top + pbuff->Height();
	}

	CAutoRefPtr<IRegion> rgn;
	GETRENDERFACTORY->CreateRegion(&rgn);
	rgn->CombineRect(rcWnd, RGN_OR);

	CAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(rcWnd, OLEDC_PAINTBKGND, FALSE);
	pRT->BitBlt(rcWnd, m_prtBackground, rcWnd.left, rcWnd.top);
	pRT->DrawBitmap(rcWnd, pbuff, 0, 0, 0xFF);
	ReleaseRenderTarget(pRT);
}

BOOL SWebView::IsCaptured() {
	return GetCapture() == GetSwnd();
}

int SWebView::GetPopupXOffset() const {
	CEF_REQUIRE_UI_THREAD();
	return original_popup_rect_.x - popup_rc.left;
}

int SWebView::GetPopupYOffset() const {
	CEF_REQUIRE_UI_THREAD();
	return original_popup_rect_.y - popup_rc.top;
}

void SWebView::ApplyPopupOffset(int& x, int& y) const {
	if (IsOverPopupWidget(x, y)) {
		x += GetPopupXOffset();
		y += GetPopupYOffset();
	}
}

bool SWebView::IsOverPopupWidget(int x, int y) const {
	CEF_REQUIRE_UI_THREAD();
	int popup_right = popup_rc.left + popup_rc.Width();
	int popup_bottom = popup_rc.top + popup_rc.Height();
	return (x >= popup_rc.left) && (x < popup_right) && (y >= popup_rc.top) && (y < popup_bottom);
}

// 
//  SWebViewHandler::MessageHandlermethods
//
bool SWebView::OnBrowserMessage(CefRefPtr<CefBrowser> browser,
								CefProcessId source_process,
								CefRefPtr<CefProcessMessage> message) {
	EventWebViewNotify evt(this);

	evt.MessageName = message->GetName().ToWString().c_str();
	CefRefPtr<CefListValue> arg = message->GetArgumentList();

	for (unsigned int i = 0; i < arg->GetSize(); ++i) {
		SStringW str = arg->GetString(i).ToWString().c_str();
		evt.Arguments.Add(str);
	}

	return !!FireEvent(evt);
}

//ClientHandlerOsr::OsrDelegate methods.

void SWebView::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();
	if (!m_strUrl.IsEmpty()) {
		LoadURL(m_strUrl);
	}
}


void SWebView::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();
	//Destroy();
}


bool SWebView::GetRootScreenRect(CefRefPtr<CefBrowser> browser,
								 CefRect& rect) {
	CEF_REQUIRE_UI_THREAD();
	return false;
}

bool SWebView::GetViewRect(CefRefPtr<CefBrowser> browser,
						   CefRect& rect) {
	CEF_REQUIRE_UI_THREAD();

	//STRACE(L"browser get view rect");
	CRect rc = GetClientRect();

	rect.x = rect.y = 0;
	rect.width = browser::DeviceToLogical(rc.Width(), device_scale_factor_);
	rect.height = browser::DeviceToLogical(rc.Height(), device_scale_factor_);
	return true;
}

bool SWebView::GetScreenPoint(CefRefPtr<CefBrowser> browser,
							  int viewX,
							  int viewY,
							  int& screenX,
							  int& screenY) {
	CEF_REQUIRE_UI_THREAD();

	// Convert the point from view coordinates to actual screen coordinates.
	POINT screen_pt = {browser::LogicalToDevice(viewX, device_scale_factor_),
		browser::LogicalToDevice(viewY, device_scale_factor_)};
	ClientToScreen(GetContainer()->GetHostHwnd(), &screen_pt);
	screenX = screen_pt.x;
	screenY = screen_pt.y;
	return true;
}


bool SWebView::GetScreenInfo(CefRefPtr<CefBrowser> browser,
							 CefScreenInfo& screen_info) {
	CEF_REQUIRE_UI_THREAD();

	if (!IsVisible())
		return false;

	CefRect view_rect;
	GetViewRect(browser, view_rect);

	screen_info.device_scale_factor = device_scale_factor_;

	// The screen info rectangles are used by the renderer to create and position
	// popups. Keep popups inside the view rectangle.
	screen_info.rect = view_rect;
	screen_info.available_rect = view_rect;
	return true;
}

void SWebView::OnPopupShow(CefRefPtr<CefBrowser> browser,
						   bool show) {
	CEF_REQUIRE_UI_THREAD();

	if (!show) {
		original_popup_rect_.Set(0, 0, 0, 0);
		resize_popup_ = false;
		CHECK_CEF_BROWSER_HOST;
		host->Invalidate(PET_VIEW);
	}
}


void SWebView::OnPopupSize(CefRefPtr<CefBrowser> browser,
						   const CefRect& rect) {
	CEF_REQUIRE_UI_THREAD();

	CefRect &rc = browser::LogicalToDevice(rect, device_scale_factor_);
	original_popup_rect_ = rc;

	CRect &rcView = GetClientRect();

	if (rc.width <= 0 || rc.height <= 0)
		return;

	if (rc.x <= 0)
		rc.x = 0;
	if (rc.y)
		rc.y = 0;
	if (rc.x + rc.width > rcView.Width())
		rc.x = rcView.Width() - rc.width;
	if (rc.y + rc.height > rcView.Height())
		rc.y = rcView.Height() - rc.height;
	if (rc.x <= 0)
		rc.x = 0;
	if (rc.y <= 0)
		rc.y = 0;
	popup_rc = CRect(CPoint(rc.x, rc.y), CSize(rc.width, rc.height));

	resize_popup_ = true;
}


void SWebView::AdjustPixmap(int width, int height) {
	if (!m_pBitmapBuff) {
		GETRENDERFACTORY->CreateBitmap(&m_pBitmapBuff);
	}

	if (m_pBitmapBuff->Width() != (unsigned int) width || m_pBitmapBuff->Height() != (unsigned int) height) {
		m_pBitmapBuff->Init(width, height);
	}
}


void SWebView::OnPaint(CefRefPtr<CefBrowser> browser,
					   CefRenderHandler::PaintElementType type,
					   const CefRenderHandler::RectList& dirtyRects,
					   const void* buffer,
					   int width,
					   int height) {
	CEF_REQUIRE_UI_THREAD();

	if (painting_popup_) {
		if (type == PET_VIEW) {

			AdjustPixmap(width, height);
			LPBYTE pDst = (LPBYTE) m_pBitmapBuff->LockPixelBits();
			if (pDst) {
				memcpy(pDst, buffer, width * height * 4);
				m_pBitmapBuff->UnlockPixelBits(pDst);
			}

			draw_popup_ = false;
		} else if (type == PET_POPUP) {
			AdjustPixmap(width, height);
			LPBYTE pDst = (LPBYTE) m_pBitmapBuff->LockPixelBits();
			if (pDst) {
				memcpy(pDst, buffer, width * height * 4);
				m_pBitmapBuff->UnlockPixelBits(pDst);
			}

			draw_popup_ = true;
		}

		return;
	}

	if (type == PET_VIEW) {
		AdjustPixmap(width, height);
		LPBYTE pDst = (LPBYTE) m_pBitmapBuff->LockPixelBits();
		if (pDst) {
			memcpy(pDst, buffer, width * height * 4);
			m_pBitmapBuff->UnlockPixelBits(pDst);
		}

		draw_popup_ = false;
	} else if (type == PET_POPUP) {

		AdjustPixmap(width, height);
		LPBYTE pDst = (LPBYTE) m_pBitmapBuff->LockPixelBits();
		if (pDst) {
			memcpy(pDst, buffer, width * height * 4);
			m_pBitmapBuff->UnlockPixelBits(pDst);
		}

		draw_popup_ = true;
	}

	if (type == PET_VIEW && resize_popup_) {
		painting_popup_ = true;
		browser->GetHost()->Invalidate(PET_POPUP);
		painting_popup_ = false;
	}


	if (m_pBitmapBuff) {
		if (draw_popup_) {
			Draw(popup_rc.left, popup_rc.top, m_pBitmapBuff);

		} else {
			Draw(0, 0, m_pBitmapBuff);
		}
	}
}

void SWebView::OnCursorChange(CefRefPtr<CefBrowser> browser,
							  CefCursorHandle cursor,
							  CefRenderHandler::CursorType type,
							  const CefCursorInfo& custom_cursor_info) {
	CEF_REQUIRE_UI_THREAD();

	if (!m_bSkipCursor) {
		::SetCursor(cursor);
	}
}

void SWebView::UpdateDragCursor(
	CefRefPtr<CefBrowser> browser,
	CefRenderHandler::DragOperation operation) {
	CEF_REQUIRE_UI_THREAD();
	m_opCurrentDrag = operation;
}

void SWebView::OnImeCompositionRangeChanged(
	CefRefPtr<CefBrowser> browser,
	const CefRange& selection_range,
	const CefRenderHandler::RectList& character_bounds) {
	CEF_REQUIRE_UI_THREAD();
	if (ime_handler_) {
		// Convert from view coordinates to device coordinates.
		CefRenderHandler::RectList device_bounds;
		CefRenderHandler::RectList::const_iterator it = character_bounds.begin();
		for (; it != character_bounds.end(); ++it) {
			device_bounds.push_back(browser::LogicalToDevice(*it, device_scale_factor_));
		}

		ime_handler_->ChangeCompositionRange(selection_range, device_bounds);
	}
}

void SWebView::OnBeforePopup(CefRefPtr<CefBrowser> browser,
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
}

void SWebView::OnAddressChange(CefRefPtr<CefBrowser> browser,
							   CefRefPtr<CefFrame> frame,
							   const CefString &url) {
	m_strUrl = url.ToWString().c_str();
	EventURLChange ev(this, m_strUrl);
	FireEvent(ev);
}

void SWebView::OnLoadEnd(CefRefPtr<CefBrowser> browser,
						 CefRefPtr<CefFrame> frame,
						 int httpStatusCode) {
	EventWebviewLoadEnd ev(this, httpStatusCode);
	FireEvent(ev);
}

}; // namespace SOUI