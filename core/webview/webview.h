#pragma once

#include "webview_handler.h"
#include "main_context.h"

namespace browser{
	class OsrImeHandlerWin;
}

namespace SOUI {

class SWebView : public SWindow
	, public browser::SWebViewHandler::OsrDelegate
	, public browser::SWebViewHandler::MessageHandler
	, public browser::IBrowserClient {
	SOUI_CLASS_NAME(SWebView, L"webview")

public:
	SWebView(void);
	~SWebView(void);

	HWND getWindowHandle() OVERRIDE {
		return this->GetContainer()->GetHostHwnd();
	}
	CefClient  *getClient() OVERRIDE {
		return webview_handler_;
	}

	void SetDevTools(bool devtools);
	// ------------------------------------------------------------------------------
	//
	// methods for user
	//
	// ------------------------------------------------------------------------------ 
	BOOL LoadURL(const SStringW& url);
	SStringW GetTitle();
	SStringW GetURL();
	void ExecJavaScript(const SStringW& js);
	BOOL CanGoBack();
	void GoBack();
	BOOL CanGoForward();
	void GoForward();
	BOOL IsLoading();
	void Reload();
	void StopLoad();
	BOOL Open();
	void Close();
	void GetWebViewSource(std::function<void(const CefString&)> &&callback);


	// ------------------------------------------------------------------------------
	//
	// browser callbacks
	//
	// ------------------------------------------------------------------------------
protected:

	//
	// callback helper methods
	//
	int GetPopupXOffset() const;
	int GetPopupYOffset() const;
	bool IsOverPopupWidget(int x, int y) const;
	void ApplyPopupOffset(int& x, int& y) const;
	void SendFocusEvent(bool focus);
	void SendMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void SendKeyEvent(UINT uMsg, WPARAM wp, LPARAM lp);
	void SendCaptureLostEvent(UINT uMsg, WPARAM wp, LPARAM lp);
	void  Draw(int x, int y, IBitmap * pbuff);
	BOOL  IsCaptured();

	//
	// ClientHandler::MessageHandler methods
	//
	virtual bool OnBrowserMessage(CefRefPtr<CefBrowser> browser,
								  CefProcessId source_process,
								  CefRefPtr<CefProcessMessage> message) OVERRIDE;

	//
	// ClientHandlerOsr::OsrDelegate methods. 
	void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
	bool GetRootScreenRect(CefRefPtr<CefBrowser> browser,
						   CefRect& rect) OVERRIDE;
	bool GetViewRect(CefRefPtr<CefBrowser> browser,
					 CefRect& rect) OVERRIDE;
	bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
						int viewX,
						int viewY,
						int& screenX,
						int& screenY) OVERRIDE;
	bool GetScreenInfo(CefRefPtr<CefBrowser> browser,
					   CefScreenInfo& screen_info) OVERRIDE;
	void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) OVERRIDE;
	void OnPopupSize(CefRefPtr<CefBrowser> browser,
					 const CefRect& rect) OVERRIDE;
	void OnPaint(CefRefPtr<CefBrowser> browser,
				 CefRenderHandler::PaintElementType type,
				 const CefRenderHandler::RectList& dirtyRects,
				 const void* buffer,
				 int width,
				 int height) OVERRIDE;
	void OnCursorChange(CefRefPtr<CefBrowser> browser,
						CefCursorHandle cursor,
						CefRenderHandler::CursorType type,
						const CefCursorInfo& custom_cursor_info) OVERRIDE;
	
	void UpdateDragCursor(CefRefPtr<CefBrowser> browser,
						  CefRenderHandler::DragOperation operation) OVERRIDE;
	void OnImeCompositionRangeChanged(
		CefRefPtr<CefBrowser> browser,
		const CefRange& selection_range,
		const CefRenderHandler::RectList& character_bounds) OVERRIDE;
	void OnBeforePopup(CefRefPtr<CefBrowser> browser,
					   CefRefPtr<CefFrame> frame,
					   const CefString& target_url,
					   const CefString& target_frame_name,
					   CefLifeSpanHandler::WindowOpenDisposition target_disposition,
					   bool user_gesture,
					   const CefPopupFeatures& popupFeatures,
					   CefWindowInfo& windowInfo,
					   CefRefPtr<CefClient>& client,
					   CefBrowserSettings& settings,
					   bool* no_javascript_access) OVERRIDE;
	void OnAddressChange(CefRefPtr<CefBrowser> browser,
						 CefRefPtr<CefFrame> frame,
						 const CefString &url) OVERRIDE;

	void OnLoadEnd(CefRefPtr<CefBrowser> browser,
							   CefRefPtr<CefFrame> frame,
							   int httpStatusCode) OVERRIDE;

protected:

	void AdjustPixmap(int width, int height);
	void UpdateBkgndRenderTarget();
	BOOL OnAttrUrl(SStringW strValue, BOOL bLoading);

	SOUI_ATTRS_BEGIN()
		ATTR_CUSTOM(L"url", OnAttrUrl)
	SOUI_ATTRS_END()

	virtual BOOL OnSetCursor(const CPoint &pt);

	int  OnCreate(LPVOID);
	void OnDestroy();
	void OnSize(UINT nType, CSize size);
	void OnPaint(IRenderTarget *pRT);
	void OnSetFocus(SWND wndOld);
	void OnKillFocus(SWND wndFocus);
	void OnShowWindow(BOOL bShow, UINT nStatus);

	LRESULT OnCaptureChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);

	//LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	LRESULT OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnIMESetContext(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnIMEStartComposition(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnIMEComposition(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnIMECancelCompositionEvent(UINT message, WPARAM wParam, LPARAM lParam);


	SOUI_MSG_MAP_BEGIN()
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		MSG_WM_PAINT_EX(OnPaint)
		MSG_WM_SETFOCUS_EX(OnSetFocus)
		MSG_WM_KILLFOCUS_EX(OnKillFocus)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MESSAGE_HANDLER_EX(WM_CAPTURECHANGED, OnCaptureChanged)
		MESSAGE_HANDLER_EX(WM_CANCELMODE, OnCaptureChanged)

		MESSAGE_HANDLER_EX(WM_IME_SETCONTEXT, OnIMESetContext)
		MESSAGE_HANDLER_EX(WM_IME_STARTCOMPOSITION, OnIMEStartComposition)
		MESSAGE_HANDLER_EX(WM_IME_COMPOSITION, OnIMEComposition)
		MESSAGE_HANDLER_EX(WM_IME_ENDCOMPOSITION, OnIMECancelCompositionEvent)

		MESSAGE_HANDLER_EX(WM_LBUTTONDOWN, OnMouseEvent)
		MESSAGE_HANDLER_EX(WM_RBUTTONDOWN, OnMouseEvent)
		MESSAGE_HANDLER_EX(WM_MBUTTONDOWN, OnMouseEvent)
		MESSAGE_HANDLER_EX(WM_LBUTTONUP, OnMouseEvent)
		MESSAGE_HANDLER_EX(WM_RBUTTONUP, OnMouseEvent)
		MESSAGE_HANDLER_EX(WM_MBUTTONUP, OnMouseEvent)
		MESSAGE_HANDLER_EX(WM_MOUSEMOVE, OnMouseEvent)
		MESSAGE_HANDLER_EX(WM_MOUSELEAVE, OnMouseEvent)
		MESSAGE_HANDLER_EX(WM_MOUSEWHEEL, OnMouseEvent)

		MESSAGE_HANDLER_EX(WM_SYSCHAR, OnKeyEvent)
		MESSAGE_HANDLER_EX(WM_SYSKEYDOWN, OnKeyEvent)
		MESSAGE_HANDLER_EX(WM_SYSKEYUP, OnKeyEvent)
		MESSAGE_HANDLER_EX(WM_KEYDOWN, OnKeyEvent)
		MESSAGE_HANDLER_EX(WM_KEYUP, OnKeyEvent)
		MESSAGE_HANDLER_EX(WM_CHAR, OnKeyEvent)
	SOUI_MSG_MAP_END()

	// ------------------------------------------------------------------------------
	//
	// members
	//
	// ------------------------------------------------------------------------------
protected:
	bool hidden_;

	bool painting_popup_;
	bool resize_popup_;
	bool draw_popup_;
	CRect popup_rc;

	BOOL                            m_bInternalPaint;
	browser::SWebViewHandler        *webview_handler_;
	SStringW                        m_strUrl;
	SStringW                        m_strTitle;
	BOOL                            m_bSkipCursor;
	BOOL                            m_bBkgndDirty;
	CAutoRefPtr<IRenderTarget>      m_prtBackground;  /**< »º´æ´°¿Ú»æÖÆµÄRT */
	CAutoRefPtr<IBitmap>            m_pBitmapBuff;
	CefRenderHandler::DragOperation m_opCurrentDrag;
	scoped_ptr<browser::OsrImeHandlerWin> ime_handler_;

	bool first_show_;
	float device_scale_factor_;

	POINT last_mouse_pos_;
	POINT current_mouse_pos_;
	bool mouse_rotation_;
	bool mouse_tracking_;
	int last_click_x_;
	int last_click_y_;
	CefBrowserHost::MouseButtonType last_click_button_;
	int last_click_count_;
	double last_click_time_;
	bool last_mouse_down_on_view_;
	CefRect original_popup_rect_;
};

}
