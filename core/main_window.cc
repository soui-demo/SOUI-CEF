#include "stdafx.h"
#include "main_window.h"

#include <memory>

namespace steambox {

SMainWnd::SMainWnd()
	:SHostWnd(L"LAYOUT:XML_MAINWND") {
}

SMainWnd::~SMainWnd() {

}

void SMainWnd::OnClose() {
	DestroyWindow();
}

void SMainWnd::OnMinimize() {
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

BOOL SMainWnd::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
	layout_inited_ = TRUE;

	return 0;
}




} //namespace steambox