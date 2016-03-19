﻿/*
http://code.google.com/p/vcardtool/
Copyright (C) 2011  Just Fancy (Just_Fancy@live.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DlgAbout.h"
#include "resource.h"
#include "common.h"

static const TCHAR szGNUInfo[] = 
{
	TEXT("This program is free software: you can redistribute it and/or modify ")
	TEXT("it under the terms of the GNU General Public License as published by ")
	TEXT("the Free Software Foundation, either version 3 of the License, or ")
	TEXT("(at your option) any later version.\r\n")
	TEXT("\r\n")
	TEXT("This program is distributed in the hope that it will be useful, ")
	TEXT("but WITHOUT ANY WARRANTY; without even the implied warranty of ")
	TEXT("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the ")
	TEXT("GNU General Public License for more details. ")
	TEXT("\r\n")
	TEXT("You should have received a copy of the GNU General Public License ")
	TEXT("along with this program.  If not, see <http://www.gnu.org/licenses/>.")
};

BOOL DlgAbout::runProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			TString buildTime(_T("Build time : "));
#ifdef UNICODE
			buildTime += atow(__DATE__);
			buildTime += L" - ";
			buildTime += atow(__TIME__);
#else
			buildTime += __DATE__;
			buildTime += " - ";
			buildTime += __TIME__;
#endif
			SetItemText(IDS_BUILT, buildTime);

			buildTime = TEXT("名片小助手 v1.1 ");
#ifdef UNICODE
			buildTime += TEXT("(UNICODE)");
#else
			buildTime += TEXT("(ANSI)");
#endif
			SetItemText(IDS_APP_VER, buildTime);

			_qqLink.init(_hinst, _hWnd);
			_qqLink.create(HwndFromId(IDS_QQ), TEXT("tencent://message/?uin=551907703"));
			_codeLink.init(_hinst, _hWnd);
			_codeLink.create(HwndFromId(IDS_EMAIL), TEXT("http://code.google.com/p/vcardtool/"));

			SetItemText(IDC_EDIT1, szGNUInfo);
			return TRUE;
		}

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(_hWnd, &ps);

			HBITMAP hBitmap = GradienBitmap(_hWnd, RGB(189, 184, 237), RGB(192, 218, 206));

			Rect rect;
			GetClientRect(&rect);
			DrawBitmap(hBitmap, hdc, rect);
			DeleteObject(hBitmap);

			EndPaint(_hWnd, &ps);
		}
		break;

	case WM_CTLCOLORSTATIC:
		if (HwndFromId(IDC_EDIT1) != HWND(lParam))
			SetBkMode(HDC(wParam), TRANSPARENT);
		if (HwndFromId(IDS_BUILT) == HWND(lParam))
			SetTextColor(HDC(wParam), RGB(240, 240, 240));

		return GetStockObject(NULL_BRUSH) != NULL;

	case WM_CLOSE:
		destroy();
		break;
	}

	return FALSE;
}
