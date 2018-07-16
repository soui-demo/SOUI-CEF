#ifndef __EVENT_CENTER_H__
#define __EVENT_CENTER_H__

#pragma once

namespace steambox {

#define EVT_PING EVT_EXTERNAL_BEGIN + 100001
#define EVT_PING_FINISHED EVT_EXTERNAL_BEGIN + 100002
#define EVT_HOT_KEY_SET EVT_EXTERNAL_BEGIN + 100003
#define EVT_SHOW_MAP EVT_EXTERNAL_BEGIN + 100004

class EventPing :public TplEventArgs<EventPing> {
	SOUI_CLASS_NAME(EventPing, L"on_ping_event")
public:
	EventPing(SObject *pSender) :TplEventArgs<EventPing>(pSender) {
	}

	enum {
		EventID = EVT_PING,
	};

	SharedProxyDataPtr proxy;
	int type_index;
};

class EventPingFinished :public TplEventArgs<EventPingFinished> {
	SOUI_CLASS_NAME(EventPingFinished, L"on_ping_finished")
public:
	EventPingFinished(SObject *pSender) :TplEventArgs<EventPingFinished>(pSender) {

	}

	enum {
		EventID = EVT_PING_FINISHED,
	};

	SharedHostItemPtr host_item;
	int index;
};

class EventSetKey :public TplEventArgs<EventSetKey> {
	SOUI_CLASS_NAME(EventSetKey, L"on_hot_key_event")
public:
	EventSetKey(SObject *pSender) :TplEventArgs<EventSetKey>(pSender) {

	}
	enum {
		EventID = EVT_HOT_KEY_SET,
	};
	WORD vKey;
	WORD wModifiers;
};

class EventShowMap :public TplEventArgs<EventShowMap> {
	SOUI_CLASS_NAME(EventShowMap, L"on_show_map_event")
public:
	EventShowMap(SObject *pSender) :TplEventArgs<EventShowMap>(pSender) {

	}
	enum {
		EventID=EVT_SHOW_MAP,
	};
	BOOL bShow;
};
}//namespace steambox


#endif