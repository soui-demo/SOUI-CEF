#pragma once


namespace SOUI
{

namespace {
enum {
	EVT_CEFWEBVIEW_BEGIN = EVT_EXTERNAL_BEGIN + 0x1000,
	EVT_WEBVIEW_NOTIFY = EVT_CEFWEBVIEW_BEGIN,
	EVT_WEBVIEW_LOAD_END,
	EVT_WEBVIEW_URL_CHANGE,
	EVT_PING_FINISHED,
	EVT_SHOW_MAP,
	EVT_FETCH_VERSION,
	EVT_SHOW_SCORE,
	EVT_DEATH_MSG,
	EVT_PLAYER_SPAWN,
	EVT_DOWNLOAD_TOOL,
	EVT_UPDATE_NAME,
	EVT_UPDATE_ROUTE,
	EVT_DRAW_ROUTE,
};
}

class EventWebViewNotify : public TplEventArgs<EventWebViewNotify> {
	SOUI_CLASS_NAME(EventWebViewNotify, L"on_webview_notify")
public:
	EventWebViewNotify(SObject *pSender)
		:TplEventArgs<EventWebViewNotify>(pSender) {
	}
	enum {
		EventID = EVT_WEBVIEW_NOTIFY
	};

	SStringW         MessageName;
	SArray<SStringW> Arguments;
};

class EventWebviewLoadEnd :public TplEventArgs<EventWebviewLoadEnd> {
	SOUI_CLASS_NAME(EventWebviewLoadEnd, L"on_webview_load_end")
public:
	EventWebviewLoadEnd(SObject *pSender, int httpcode)
		:TplEventArgs<EventWebviewLoadEnd>(pSender)
		, httpcode_(httpcode) {

	}

	enum {
		EventID = EVT_WEBVIEW_LOAD_END,
	};
	int httpcode_;
};

class EventURLChange :public TplEventArgs<EventURLChange> {
	SOUI_CLASS_NAME(EventURLChange, L"on_web_url_change");
public:
	EventURLChange(SObject *pSender, const SStringW &url)
		:TplEventArgs<EventURLChange>(pSender), url_(url) {

	}
	enum {
		EventID = EVT_WEBVIEW_URL_CHANGE,
	};

	SStringW url_;
};

}// namespace SOUI