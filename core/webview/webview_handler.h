#ifndef _BROWSER_BROWSERHANDLER_H
#define _BROWSER_BROWSERHANDLER_H

#pragma once

#include "include/cef_client.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "client_app.h"
#include "include/cef_drag_handler.h"
#include "include/wrapper/cef_resource_manager.h"
 
#include <list>
#include "souistd.h"

#pragma once


namespace browser
{  

const int POPUP_DEVTOOLS = WM_APP + 1;

class SWebViewHandler : public CefClient,
                       public CefRenderHandler,
                       public CefContextMenuHandler,
                       public CefDisplayHandler,
                       public CefDragHandler,
                       public CefKeyboardHandler,
                       public CefLifeSpanHandler,
					   public CefLoadHandler,
					   public CefRequestHandler
{
public: 
	// Implement this interface to receive notification of ClientHandlerOsr
	// events. The methods of this class will be called on the CEF UI thread.
	class OsrDelegate {
	public:
		// These methods match the CefLifeSpanHandler interface.
		virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) = 0;
		virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) = 0;

		// These methods match the CefRenderHandler interface.
		virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser,
			CefRect& rect) = 0;
		virtual bool GetViewRect(CefRefPtr<CefBrowser> browser,
			CefRect& rect) = 0;
		virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
			int viewX,
			int viewY,
			int& screenX,
			int& screenY) = 0;
		virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser,
			CefScreenInfo& screen_info) = 0;
		virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) = 0;
		virtual void OnPopupSize(CefRefPtr<CefBrowser> browser,
			const CefRect& rect) = 0;
		virtual void OnPaint(CefRefPtr<CefBrowser> browser,
			PaintElementType type,
			const RectList& dirtyRects,
			const void* buffer,
			int width,
			int height) = 0;
		virtual void OnCursorChange(
			CefRefPtr<CefBrowser> browser,
			CefCursorHandle cursor,
			CefRenderHandler::CursorType type,
			const CefCursorInfo& custom_cursor_info) = 0;
		virtual void UpdateDragCursor(
			CefRefPtr<CefBrowser> browser,
			CefRenderHandler::DragOperation operation) = 0;
		virtual void OnImeCompositionRangeChanged(
			CefRefPtr<CefBrowser> browser,
			const CefRange& selection_range,
			const CefRenderHandler::RectList& character_bounds) = 0;

		virtual void OnBeforePopup(CefRefPtr<CefBrowser> browser,
								   CefRefPtr<CefFrame> frame,
								   const CefString& target_url,
								   const CefString& target_frame_name,
								   CefLifeSpanHandler::WindowOpenDisposition target_disposition,
								   bool user_gesture,
								   const CefPopupFeatures& popupFeatures,
								   CefWindowInfo& windowInfo,
								   CefRefPtr<CefClient>& client,
								   CefBrowserSettings& settings,
								   bool* no_javascript_access) = 0;

		virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,
									 CefRefPtr<CefFrame> frame,
									 const CefString &url) = 0;

		virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
							   CefRefPtr<CefFrame> frame,
							   int httpStatusCode) = 0;
	protected:
		virtual ~OsrDelegate() {}
	};


    class MessageHandler
    {
    public:
        virtual bool OnBrowserMessage(CefRefPtr<CefBrowser> browser,
            CefProcessId source_process,
            CefRefPtr<CefProcessMessage> message) = 0;
    };

    SWebViewHandler();
    ~SWebViewHandler(); 

// ------------------------------------------------------------------------------
// 
// browser methods for user
//
// ------------------------------------------------------------------------------

    IMPLEMENT_REFCOUNTING(SWebViewHandler);

public:
	void CloseDevTools(CefRefPtr<CefBrowser> browser); 

    bool CreatePopupWindow(
		CefRefPtr<CefBrowser> browser,
		bool is_devtools,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings);
	 

    BOOL Open(HWND hParent, SOUI::CRect rcView);
    void Close();
    void CloseAllBrowsers(bool force_close);
    void OpenDevTools();
	void SetRender(OsrDelegate * pHost);
    CefRefPtr<CefBrowser> GetBrowser();
    void RegisterMessageHandler(MessageHandler * handler);
    void UnRegisterMessgeHandler(MessageHandler * handler);

// ------------------------------------------------------------------------------
//
// impl cef handler callbacks
//
// ------------------------------------------------------------------------------

protected:

    //
    // CefClient methods. Important to return |this| for the handler callbacks.
    // 

    virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE { return this; }
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler()           OVERRIDE { return this; }
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler()         OVERRIDE { return this; }
    virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler()       OVERRIDE { return this; }
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler()       OVERRIDE { return this; }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler()               OVERRIDE { return this; }
    virtual CefRefPtr<CefDragHandler> GetDragHandler()               OVERRIDE { return this; }

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message)  OVERRIDE 
    {
        if (m_pMsgHandler)
        {
            return m_pMsgHandler->OnBrowserMessage(browser, source_process, message);
        }

        return true;
    }

	// CefRequestHandler methods
	bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		bool is_redirect) OVERRIDE;
	bool OnOpenURLFromTab(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& target_url,
		CefRequestHandler::WindowOpenDisposition target_disposition,
		bool user_gesture) OVERRIDE;
	cef_return_value_t OnBeforeResourceLoad(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		CefRefPtr<CefRequestCallback> callback) OVERRIDE;
	CefRefPtr<CefResourceHandler> GetResourceHandler(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request) OVERRIDE;
	CefRefPtr<CefResponseFilter> GetResourceResponseFilter(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		CefRefPtr<CefResponse> response) OVERRIDE;
	bool OnQuotaRequest(CefRefPtr<CefBrowser> browser,
		const CefString& origin_url,
		int64 new_size,
		CefRefPtr<CefRequestCallback> callback) OVERRIDE;
	void OnProtocolExecution(CefRefPtr<CefBrowser> browser,
		const CefString& url,
		bool& allow_os_execution) OVERRIDE;
	bool OnCertificateError(
		CefRefPtr<CefBrowser> browser,
		ErrorCode cert_error,
		const CefString& request_url,
		CefRefPtr<CefSSLInfo> ssl_info,
		CefRefPtr<CefRequestCallback> callback) OVERRIDE;
	void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
		TerminationStatus status) OVERRIDE;

	 
    // CefRenderHandler interfaces 
    virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser,
		CefRect& rect) OVERRIDE;
	virtual bool GetViewRect(CefRefPtr<CefBrowser> browser,
		CefRect& rect) OVERRIDE;
	virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
		int viewX,
		int viewY,
		int& screenX,
		int& screenY) OVERRIDE;
	virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser,
		CefScreenInfo& screen_info)  OVERRIDE;
    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser,
        bool show) OVERRIDE; 
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser,
        const CefRect& rect)  OVERRIDE; 
    virtual void OnPaint(
        CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const void* buffer,
        int width,
        int height) OVERRIDE; 
	virtual void OnCursorChange(CefRefPtr<CefBrowser> browser,
		CefCursorHandle cursor,
		CursorType type,
		const CefCursorInfo& custom_cursor_info) OVERRIDE; 
    virtual bool StartDragging(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDragData> drag_data,
        CefRenderHandler::DragOperationsMask allowed_ops,
        int x, int y) OVERRIDE; 
    virtual void UpdateDragCursor(
        CefRefPtr<CefBrowser> browser,
        CefRenderHandler::DragOperation operation)
        OVERRIDE;
	 void OnImeCompositionRangeChanged(
      CefRefPtr<CefBrowser> browser,
      const CefRange& selection_range,
      const CefRenderHandler::RectList& character_bounds) OVERRIDE;

    //
    // CefContextMenuHandler methods
    // 
	virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefContextMenuParams> params,
		CefRefPtr<CefMenuModel> model) OVERRIDE;

	// CefDisplayHandler methods
	void OnTitleChange(CefRefPtr<CefBrowser> browser,
					   const CefString& title) OVERRIDE;

	bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
						  const CefString& message,
						  const CefString& source, int line) OVERRIDE;
	void OnAddressChange(CefRefPtr<CefBrowser> browser,
						 CefRefPtr<CefFrame> frame,
						 const CefString& url) OVERRIDE;


    //
    // CefKeyboardHandler methods
    //
    virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
        const CefKeyEvent& event,
        CefEventHandle os_event,
        bool* is_keyboard_shortcut) OVERRIDE;

    //
    // CefLifeSpanHandler methods
    //
	virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& target_url,
		const CefString& target_frame_name,
		CefLifeSpanHandler::WindowOpenDisposition target_disposition,
		bool user_gesture,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings,
		bool* no_javascript_access)OVERRIDE;

    virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE ;
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser)OVERRIDE;

    //
    // CefLoadHandler methods
    //

    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
							 CefRefPtr<CefFrame> frame,
							 TransitionType transition_typ) OVERRIDE
    {
    }

    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        int httpStatusCode) OVERRIDE
    {
        m_bPageLoaded = TRUE;
		if (m_posr) {
			m_posr->OnLoadEnd(browser, frame, httpStatusCode);
		}
    }

    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        ErrorCode errorCode,
        const CefString& errorText,
        const CefString& failedUrl)
    {
    }


private:
    CefRefPtr<CefBrowser>   m_refBrowser;
    BOOL                    m_bPageLoaded;
	OsrDelegate       * m_posr;
    MessageHandler        * m_pMsgHandler;

	CefRefPtr<CefClient>    m_refDevToolsClient;
	// Manages the registration and delivery of resources.
	CefRefPtr<CefResourceManager> resource_manager_;

	// Console logging state.
	const std::string console_log_file_;

	bool first_console_message_;
};


} //namespace browser

#endif