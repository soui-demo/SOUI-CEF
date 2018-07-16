#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__
#pragma once

#include "webview\webview.h"
#include "extend_events.h"
#include <thread>

namespace steambox {

	class SMainWnd :public SHostWnd,public TAutoEventMapReg<SMainWnd> {
	public:
		enum {
			SWITCH_ANIMATION_SIZE = 5,
			SWITCH_HIDE_TIMERID = 101,
			SWITCH_SHOW_TIMERID,
			REFRESH_STATS_TIMERID,
		};

		SMainWnd();
		~SMainWnd();

		void OnClose();
		void OnMinimize();
		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

		EVENT_MAP_BEGIN()
			EVENT_ID_COMMAND(R.id.btn_close, OnClose)
			EVENT_ID_COMMAND(R.id.btn_min, OnMinimize)
		EVENT_MAP_END()

		BEGIN_MSG_MAP_EX(SMainWnd)
			MSG_WM_INITDIALOG(OnInitDialog)
			MSG_WM_CLOSE(OnClose)
			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()

	private:
		BOOL layout_inited_ = FALSE;
	};

} //namespace steambox

#endif // __MAIN_WINDOW_H__