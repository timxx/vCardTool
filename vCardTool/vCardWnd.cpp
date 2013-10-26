/*
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

//////////////////////////////////////////////////////////////////////////
#include <Windowsx.h>
#include <Shlwapi.h>
#include <shlobj.h>

#include "vCardWnd.h"
#include "Tim\ClassEx.h"
#include "DlgAbout.h"
#include "Tim\SException.h"
#include "common.h"
#include "RegHelper.h"
#include "Tim\File.h"
//////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "shlwapi.lib")
//////////////////////////////////////////////////////////////////////////
TString vCardWnd::_clsName = TEXT("vCardToolClass");
//////////////////////////////////////////////////////////////////////////
WNDPROC _lpfnOldLBProc = NULL;
//////////////////////////////////////////////////////////////////////////
static LRESULT CALLBACK LBProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_RBUTTONUP)
	{
		NMHDR nmhdr = {0};
		nmhdr.code = NM_RCLICK;
		nmhdr.hwndFrom = hWnd;

		SendMessage(GetParent(hWnd), WM_NOTIFY, 0, (LPARAM)&nmhdr);
	}

	return CallWindowProc(_lpfnOldLBProc, hWnd, uMsg, wParam, lParam);
}
//========================================================================
void vCardWnd::init(HINSTANCE hinst, HWND hwndParent)
{
	Window::init(hinst, hwndParent);

	ClassEx cls(hinst, _clsName.c_str(), IDI_APP_MAIN, IDR_MENU_MAIN, WndProc);

	if (!cls.Register())
		throw std::runtime_error("vCardWnd::Register Failed");

	_hWnd = createEx(WS_EX_ACCEPTFILES, _clsName.c_str(), _strCaption,
		WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, DEF_CXWND, DEF_CYWND, 0, (LPVOID)this);

	if (!_hWnd)	throw std::runtime_error("vCardWnd::CreateWindowEx Failed");

	centerWnd();

	showWindow();
	UpdateWindow();

	InitializeCriticalSection(&_cs);
}
//========================================================================
LRESULT CALLBACK vCardWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_NCCREATE :
		{
			vCardWnd *pWnd = (vCardWnd *)(((LPCREATESTRUCT)lParam)->lpCreateParams);
			pWnd->_hWnd = hWnd;
			::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWnd);
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

	default:
		return ((vCardWnd *)::GetWindowLongPtr(hWnd, GWL_USERDATA))->runProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}
//========================================================================
LRESULT CALLBACK vCardWnd::runProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CLOSE:
		destroy();
		break;

	case WM_DESTROY:
		if (_hFontChild){
			DeleteObject(_hFontChild);
		}
		if (_hbmp){
			DeleteObject(_hbmp);
		}
		if (_hdcMem){
			DeleteDC(_hdcMem);
		}

		DeleteCriticalSection(&_cs);

		PostQuitMessage(0);

		break;

	case WM_CREATE:
		OnCreate((LPCREATESTRUCT)lParam);
		break;

	case WM_COMMAND:
		OnCommand(LOWORD(wParam), HWND(lParam), HIWORD(wParam));
		break;

	case WM_NOTIFY:
		OnNotify(int(wParam), (NMHDR*)lParam);
		break;

	case WM_MENUSELECT:
		{
			UINT id[] = {0, IDR_MENU_MAIN};
			MenuHelp(uMsg, wParam, lParam, GetMenu(), _hinst, _status.getSelf(), id);
		}
		break;

	case WM_SIZE:
		OnSize(int(wParam), LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_PAINT:
		OnPaint();
		break;

	case WM_CTLCOLORSTATIC:
		SetBkMode(HDC(wParam), TRANSPARENT);
		SetTextColor(HDC(wParam), RGB(128, 128, 128));
		return (LRESULT)GetStockObject(NULL_BRUSH);

// 	case WM_GETMINMAXINFO:
// 		{
// 			MINMAXINFO *pmmi = (MINMAXINFO*)lParam;
// 			pmmi->ptMinTrackSize.x = DEF_CXWND;
// 			pmmi->ptMinTrackSize.y = DEF_CYWND;
// 		}
// 		break;

	case WM_DROPFILES:
		OnDropFiles(HDROP(wParam));
		break;

	case WM_COPYDATA:
		{
			PCOPYDATASTRUCT pCDS = (PCOPYDATASTRUCT)lParam;

			if (pCDS->dwData == 0)
				OpenFromCmd((TCHAR*)pCDS->lpData);
		}
		break;

	case WM_TIMER:
		OnTimer(UINT_PTR(wParam));
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}
//========================================================================
void vCardWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	_hbmp = GradienBitmap(_hWnd, RGB(189, 184, 237), RGB(192, 218, 206));

	_vCardList.setParent(getSelf());
	_vCardList.setStyle(WS_TABSTOP | WS_VSCROLL | LBS_NOTIFY);
	_vCardList.create();
	_status.create(NULL, getSelf());

	_sInfo.init(_hinst, _hWnd);
	_sInfo.create(TEXT("( 0/0 )"));

	_lnameEdit.setParent(_hWnd);
	_lnameEdit.setStyle(WS_TABSTOP | ES_AUTOHSCROLL);
	_lnameEdit.create(TEXT(""));

	_fnameEdit.setParent(_hWnd);
	_fnameEdit.setStyle(WS_TABSTOP | ES_AUTOHSCROLL);
	_fnameEdit.create(TEXT(""));

	_fnEdit.setParent(_hWnd);
	_fnEdit.setStyle(WS_TABSTOP | ES_AUTOHSCROLL);
	_fnEdit.create(TEXT(""));

	_nnEdit.setParent(_hWnd);
	_nnEdit.setStyle(WS_TABSTOP | ES_AUTOHSCROLL);
	_nnEdit.create(TEXT(""));

	_telEdit.setParent(_hWnd);
	_telEdit.setStyle(WS_TABSTOP | ES_AUTOHSCROLL);
	_telEdit.create(TEXT(""));

	_noteEdit.setParent(_hWnd);
	_noteEdit.setStyle(WS_TABSTOP | ES_AUTOHSCROLL);
	_noteEdit.create(TEXT(""));

	_mailEdit.setParent(_hWnd);
	_mailEdit.setStyle(WS_TABSTOP | ES_AUTOHSCROLL);
	_mailEdit.create(TEXT(""));

	_titleEdit.setParent(_hWnd);
	_titleEdit.setStyle(WS_TABSTOP | ES_AUTOHSCROLL);
	_titleEdit.create(TEXT(""));

	_urlEdit.setParent(_hWnd);
	_urlEdit.setStyle(WS_TABSTOP | ES_AUTOHSCROLL);
	_urlEdit.create(TEXT(""));

	_bdEdit.setParent(_hWnd);
	_bdEdit.setStyle(WS_TABSTOP | ES_AUTOHSCROLL);
	_bdEdit.create(TEXT(""));

	_orgEdit.setParent(_hWnd);
	_orgEdit.setStyle(WS_TABSTOP | ES_AUTOHSCROLL);
	_orgEdit.create(TEXT(""));

	_addrEdit.setParent(_hWnd);
	_addrEdit.setStyle(WS_TABSTOP | ES_AUTOHSCROLL);
	_addrEdit.create(TEXT(""));

	_edFilter.init(getHinst(), getSelf());
	_edFilter.create(TEXT(""));

	_hFontChild = CreateFont(13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("宋体"));

	_vCardList.setFont(_hFontChild);
	_lnameEdit.setFont(_hFontChild);
	_fnameEdit.setFont(_hFontChild);
	_fnEdit.setFont(_hFontChild);
	_nnEdit.setFont(_hFontChild);
	_telEdit.setFont(_hFontChild);
	_mailEdit.setFont(_hFontChild);
	_noteEdit.setFont(_hFontChild);
	_addrEdit.setFont(_hFontChild);
	_orgEdit.setFont(_hFontChild);
	_bdEdit.setFont(_hFontChild);
	_urlEdit.setFont(_hFontChild);
	_titleEdit.setFont(_hFontChild);
	_edFilter.setFont(_hFontChild);
	_sInfo.setFont(_hFontChild);

	_lpfnOldLBProc = _vCardList.Subclass(LBProc);

	int aWidths[] = {DEF_CXWND*3/4, -1};
	_status.SetParts(2, aWidths);

	_status.setText(1, TEXT("\t名片小助手 v1.1"));
}
//========================================================================
void vCardWnd::OnSize(int type, int nWidth, int nHeigth)
{
	_status.resizeTo(nWidth, 20);

	_sInfo.moveTo(70, 15);
	_sInfo.resizeTo(100, 15);

	_edFilter.resizeTo(nWidth*2/5, EDIT_HEIGHT);
	_edFilter.moveTo(5, 30);

	_vCardList.resizeTo(nWidth*2/5, nHeigth - 15 - _status.getHeight() - _edFilter.getHeight() - 35);
	_vCardList.moveTo(5, _edFilter.getHeight() + 35);

	_lnameEdit.resizeTo(EDIT_WIDTH, EDIT_HEIGHT);
	_fnameEdit.resizeTo(EDIT_WIDTH, EDIT_HEIGHT);
	_fnEdit.resizeTo(EDIT_WIDTH, EDIT_HEIGHT);
	_nnEdit.resizeTo(EDIT_WIDTH, EDIT_HEIGHT);
	_telEdit.resizeTo(EDIT_WIDTH * 2+ EDIT_SPACE, EDIT_HEIGHT);
	_noteEdit.resizeTo(EDIT_WIDTH * 2+ EDIT_SPACE, EDIT_HEIGHT);
	_mailEdit.resizeTo(EDIT_WIDTH * 2+ EDIT_SPACE, EDIT_HEIGHT);
	_titleEdit.resizeTo(EDIT_WIDTH * 2+ EDIT_SPACE, EDIT_HEIGHT);
	_urlEdit.resizeTo(EDIT_WIDTH * 2+ EDIT_SPACE, EDIT_HEIGHT);
	_bdEdit.resizeTo(EDIT_WIDTH * 2+ EDIT_SPACE, EDIT_HEIGHT);
	_orgEdit.resizeTo(EDIT_WIDTH * 2+ EDIT_SPACE , EDIT_HEIGHT);
	_addrEdit.resizeTo(EDIT_WIDTH * 2+ EDIT_SPACE, EDIT_HEIGHT);

	int x = (nWidth*3/5 - EDIT_WIDTH * 2 - EDIT_SPACE)/2 + nWidth*2/5 + 10;
	int y = 10+20;

	_lnameEdit.moveTo(x, y);
	_fnameEdit.moveTo(x + EDIT_WIDTH + 10, y);

	y += EDIT_HEIGHT + EDIT_SPACE * 2;

	_fnEdit.moveTo(x, y);
	_nnEdit.moveTo(x + EDIT_WIDTH + EDIT_SPACE, y);
	y += EDIT_HEIGHT + EDIT_SPACE * 2;
	_telEdit.moveTo(x, y);
	y += EDIT_HEIGHT + EDIT_SPACE * 2;
	_bdEdit.moveTo(x, y);
	y += EDIT_HEIGHT + EDIT_SPACE * 2;
	_addrEdit.moveTo(x, y);
	y += EDIT_HEIGHT + EDIT_SPACE * 2;
	_noteEdit.moveTo(x, y);
	y += EDIT_HEIGHT + EDIT_SPACE * 2;
	_mailEdit.moveTo(x, y);
	y += EDIT_HEIGHT + EDIT_SPACE * 2;
	_urlEdit.moveTo(x, y);
	y += EDIT_HEIGHT + EDIT_SPACE * 2;
	_orgEdit.moveTo(x, y);
	y += EDIT_HEIGHT + EDIT_SPACE * 2;
	_titleEdit.moveTo(x, y);
}
//========================================================================
void vCardWnd::OnPaint()
{
	HDC hdc;
	PAINTSTRUCT ps;
	Rect rect;
	POINT pt;
	SIZE size;

	hdc = BeginPaint(_hWnd, &ps);

	if (!_fDrew)	//窗口大小是固定的，这样可以避免每次都画一次标题
	{
		_hdcMem = CreateCompatibleDC(hdc);

		SelectObject(_hdcMem, _hbmp);

		SetTextColor(_hdcMem, RGB(128, 128, 128));
		SetBkMode(_hdcMem, TRANSPARENT);

		SelectObject(_hdcMem, _hFontChild);

		GetTextExtentPoint(_hdcMem, TEXT("字体Info"), lstrlen(TEXT("字体Info")), &size);

		_lnameEdit.GetWindowRect(&rect);
		pt.x = rect.left;
		pt.y = rect.top - 2 - size.cy;
		ScreenToClient(&pt);

		TextOut(_hdcMem, 5, pt.y, TEXT("名片列表"), lstrlen(TEXT("名片列表")));

		TextOut(_hdcMem, pt.x, pt.y, TEXT("姓"), lstrlen(TEXT("姓")));

		pt.x +=  EDIT_WIDTH + 10;
		TextOut(_hdcMem, pt.x, pt.y, TEXT("名"), lstrlen(TEXT("名")));

		_fnEdit.GetWindowRect(&rect);
		pt.x = rect.left;
		pt.y = rect.top - 2 - size.cy;
		ScreenToClient(&pt);
		TextOut(_hdcMem, pt.x, pt.y, TEXT("正式姓名"), lstrlen(TEXT("正式姓名")));

		pt.x +=  EDIT_WIDTH + EDIT_SPACE;
		TextOut(_hdcMem, pt.x, pt.y, TEXT("昵称"), lstrlen(TEXT("昵称")));

		_telEdit.GetWindowRect(&rect);
		pt.x = rect.left;
		pt.y = rect.top - 2 - size.cy;
		ScreenToClient(&pt);
		TextOut(_hdcMem, pt.x, pt.y, TEXT("号码"), lstrlen(TEXT("号码")));

		_noteEdit.GetWindowRect(&rect);
		pt.x = rect.left;
		pt.y = rect.top - 2 - size.cy;
		ScreenToClient(&pt);
		TextOut(_hdcMem, pt.x, pt.y, TEXT("注释"), lstrlen(TEXT("注释")));

		_mailEdit.GetWindowRect(&rect);
		pt.x = rect.left;
		pt.y = rect.top - 2 - size.cy;
		ScreenToClient(&pt);
		TextOut(_hdcMem, pt.x, pt.y, TEXT("电子邮件"), lstrlen(TEXT("电子邮件")));

		_bdEdit.GetWindowRect(&rect);
		pt.x = rect.left;
		pt.y = rect.top - 2 - size.cy;
		ScreenToClient(&pt);
		TextOut(_hdcMem, pt.x, pt.y, TEXT("生日"), lstrlen(TEXT("生日")));

		_addrEdit.GetWindowRect(&rect);
		pt.x = rect.left;
		pt.y = rect.top - 2 - size.cy;
		ScreenToClient(&pt);
		TextOut(_hdcMem, pt.x, pt.y, TEXT("通讯地址"), lstrlen(TEXT("通讯地址")));

		_urlEdit.GetWindowRect(&rect);
		pt.x = rect.left;
		pt.y = rect.top - 2 - size.cy;
		ScreenToClient(&pt);
		TextOut(_hdcMem, pt.x, pt.y, TEXT("网址"), lstrlen(TEXT("网址")));

		_orgEdit.GetWindowRect(&rect);
		pt.x = rect.left;
		pt.y = rect.top - 2 - size.cy;
		ScreenToClient(&pt);
		TextOut(_hdcMem, pt.x, pt.y, TEXT("公司"), lstrlen(TEXT("公司")));

		_titleEdit.GetWindowRect(&rect);
		pt.x = rect.left;
		pt.y = rect.top - 2 - size.cy;
		ScreenToClient(&pt);
		TextOut(_hdcMem, pt.x, pt.y, TEXT("职务"), lstrlen(TEXT("职务")));

		_fDrew = true;
	}

	GetClientRect(&rect);

	BitBlt(hdc, 0, 0, rect.Width(), rect.Height(), _hdcMem, 0, 0, SRCCOPY);

	EndPaint(_hWnd, &ps);
}
//========================================================================
void vCardWnd::OnCommand(int id, HWND hwndCtl, UINT uNotifyCode)
{
	if (uNotifyCode == LBN_SELCHANGE && hwndCtl == _vCardList.getSelf())
	{
		doLBSelChange();
		return ;
	}
	
	if (uNotifyCode == EN_CHANGE && hwndCtl == _edFilter.getSelf())
	{
		FiltervCard();
		return ;
	}

	switch(id)
	{
	case IDM_OPEN:
		doOpen();
		break;

	case IDM_EXIT:
		sendMsg(WM_CLOSE);
		break;

	case IDM_SAVE_CUR:
	case IDM_SAVE_ALL:
		SaveVcfFile(id == IDM_SAVE_ALL);
		break;

	case IDM_CLEAR:
		if (msgBox(TEXT("真的要清空当前所有名片吗？"),
			TEXT("请确认"),
			MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			_vCardList.ResetContent();
			std::vector<vCard>().swap(_vCard);
			std::vector<vCard>().swap(_vCardFilter);

			FiltervCard();
		}
		break;

	case IDM_ABOUT:
		{
			DlgAbout about;
			about.init(getHinst(), getSelf());
			about.doModal(IDD_ABOUT);
		}
		break;

	case IDM_HELP:
		msgBox(TEXT("额，功能挺少的，就不写帮助了\r\n")
			TEXT("真有问题可以联系我 ^_^"), _strCaption, MB_ICONINFORMATION);
		break;

	case IDM_REGISTER:
	case IDM_UNREGISTER:
		RegisteFileType(id == IDM_REGISTER);
		break;
	}

}
//========================================================================
void vCardWnd::OnNotify(int id, NMHDR *pnmh)
{
	switch(pnmh->code)
	{
	case NM_RCLICK:
		PopupRKeyMenu();
		break;
	}
}
//========================================================================
void vCardWnd::OnDropFiles(HDROP hDrop)
{
	int count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

	for (int i= 0; i<count; i++)
	{
		int len = DragQueryFile(hDrop, i, NULL, 0);

		TCHAR *filePath = new TCHAR[len+1];
		TCHAR *ext = NULL;

		DragQueryFile(hDrop, i, filePath, len+1);

		ext = PathFindExtension(filePath);
		if (ext)
		{
			if (StrCmpI(ext, TEXT(".nbu"))==0 ){
				OpenNbuFile(filePath);
			}else if(StrCmpI(ext, TEXT(".vcf"))==0){
				OpenVcfFile(filePath);
			}else{
				ShowInfo(TEXT("\t请选择*.vcf、*.nbu文件！"));
			}
		}

		delete [] filePath;
	}

	if (_vCardList.GetCurSel() == LB_ERR && _vCardFilter.size()>0)
	{
		_vCardList.SetCurSel(0);
		sendMsg(WM_COMMAND, MAKEWPARAM(0, LBN_SELCHANGE), (LPARAM)_vCardList.getSelf());
	}

	DragFinish(hDrop);
}
//========================================================================
void vCardWnd::OnTimer(UINT_PTR id)
{
	if (id != TIMER_INFO_ID)
		return ;

	if (_uTimeElapsed == 7){
		KillTimer(TIMER_INFO_ID);
		_status.setText(0, TEXT(""));
	}
	else{
		_uTimeElapsed++;
	}
}
//========================================================================
void vCardWnd::doOpen()
{
	OPENFILENAME ofn = {0};
	TCHAR filePath[MAX_PATH*50] = {0};

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.Flags = OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_EXPLORER;
	ofn.hInstance = getHinst();
	ofn.hwndOwner = getSelf();
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = MAX_PATH*50;
	ofn.lpstrTitle = TEXT("请选择支持的文件");
	ofn.lpstrFilter = 
		TEXT("vCard、NBU文件(*.vcf;*.nbu)\0*.vcf;*.nbu\0")
		TEXT("vCard文件(*.vcf)\0*.vcf\0")
		TEXT("NOKIA nbu文件(*.nbu)\0*.nbu\0\0");

	if (!GetOpenFileName(&ofn))
		return ;

	_pszFile = filePath;

	_hThread = CreateThread(0, 0, HandleFilesThread, this, 0, NULL);
}
//========================================================================
void vCardWnd::doLBSelChange()
{
	int index = _vCardList.GetCurSel();
	if (index == LB_ERR || (size_t)index >= _vCardFilter.size())
		return ;

	_lnameEdit.clear();
	_fnameEdit.clear();
	_fnEdit.clear();
	_telEdit.clear();
	_noteEdit.clear();
	_mailEdit.clear();
	_addrEdit.clear();
	_orgEdit.clear();
	_urlEdit.clear();
	_nnEdit.clear();
	_titleEdit.clear();
	_bdEdit.clear();

	TString tmp = _vCardFilter[index].name();
	int pos = tmp.find(TEXT(";"));

	if (pos != TString::npos)
	{
		_lnameEdit.setText(tmp.substr(0, pos).c_str());
		_fnameEdit.setText(tmp.substr(pos+1, -1).c_str());
	}
	else{
		_lnameEdit.setText(tmp);
	}

	_fnEdit.setText(_vCardFilter[index].formattedname());
	_telEdit.setText(_vCardFilter[index].tel());
	_noteEdit.setText(_vCardFilter[index].note());
	_mailEdit.setText(_vCardFilter[index].mail());;
	_addrEdit.setText(_vCardFilter[index].address());
	_orgEdit.setText(_vCardFilter[index].organization());
	_urlEdit.setText(_vCardFilter[index].url());
	_nnEdit.setText(_vCardFilter[index].nickname());
	_titleEdit.setText(_vCardFilter[index].title());
	_bdEdit.setText(_vCardFilter[index].birthday());
}
//========================================================================
bool vCardWnd::OpenNbuFile(const TString &filePath)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	bool fOk = false;

	hFile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		goto _exit;

	DWORD dwFileSize = GetFileSize(hFile, NULL);

	if (dwFileSize < 1)
		goto _exit;

	char *buf = new char[dwFileSize+1];
	DWORD dwRead = 0;

	SecureZeroMemory(buf, dwFileSize+1);

	if (!ReadFile(hFile, buf, dwFileSize, &dwRead, NULL))
		goto _exit;

	AnalyseNbu(buf, dwFileSize);

	fOk = true;

_exit:
	if (hFile){
		CloseHandle(hFile);
	}

	if (buf){
		delete [] buf;
	}

	return fOk;
}
//========================================================================
void vCardWnd::AnalyseNbu(const char *buf, DWORD dwSize)
{
	const char *p = buf;
	DWORD i = 0;

	while (i < dwSize)
	{
		if (IsvCardBegin(p))
		{
			string vbuf;
			while (i < dwSize && !IsvCardEnd(p))
			{
				vbuf.push_back(*p);
				p++, i++;
			}

			if (IsvCardEnd(p))
			{
				while (i < dwSize && *p != 'R')
				{
					vbuf.push_back(*p);
					p++, i++;
				}


				if (i >= dwSize)
					break;

				vbuf.push_back(*p);
				p++, i++;

				if (i >= dwSize)
					break;

				vbuf.push_back(*p);
				p++, i++;

				try
				{
					vCard card(vbuf);

					EnterCriticalSection(&_cs);

					_vCard.push_back(card);

					LeaveCriticalSection(&_cs);
				}
				catch(exception &e)
				{
					MessageBoxA(_hWnd, e.what(), "C++ Exception", MB_ICONERROR);
				}

			}
		}

		if (i >= dwSize)
			break;

		p++, i++;
	}

	//_vCardFilter = _vCard;
	FiltervCard();
}
//========================================================================
bool vCardWnd::IsvCardBegin(const char *buf)
{
	const char *p = buf;
	const char *bv = "BEGIN:VCARD";

	while (*bv && *p && *bv == *p)
		bv++, p++;

	if (!*bv){
		return true;
	}

	return false;
}
//========================================================================
bool vCardWnd::IsvCardEnd(const char *buf)
{
	const char *p = buf;
	const char *ev = "END:VCARD";

	while (*ev && *p && *ev == *p)
		ev++, p++;

	if (!*ev){
		return true;
	}

	return false;
}
//========================================================================
bool vCardWnd::OpenVcfFile(const TString &filePath)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	bool fOk = false;

	hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		goto _exit;

	DWORD dwFileSize = GetFileSize(hFile, NULL);

	if (dwFileSize < 1)
		goto _exit;

	char *buf = new char[dwFileSize+1];
	DWORD dwRead = 0;

	SecureZeroMemory(buf, dwFileSize+1);

	if (!ReadFile(hFile, buf, dwFileSize, &dwRead, NULL))
		goto _exit;

	{
		vCard card(buf);

		EnterCriticalSection(&_cs);

		_vCard.push_back(card);
		//_vCardList.AddString(card.name());

		LeaveCriticalSection(&_cs);
	}

	fOk = true;

_exit:
	if (hFile){
		CloseHandle(hFile);
	}
	if (buf){
		delete [] buf;
	}

	//_vCardFilter = _vCard;

	FiltervCard();

	return fOk;
}
//========================================================================
void vCardWnd::SaveVcfFile(bool fAll /* = false */)
{
	TString path;

	if (_vCardList.GetCount() > 1 && fAll)
	{
		path = SelectFolder();
		if (path.empty())
			return ;

		TString tmp;
		TString name;
		for (int i=0; i<(int)_vCardFilter.size(); i++)
		{
			tmp = path;
			if (tmp.at(tmp.length()-1) != TEXT('\\'))
				tmp.append(TEXT("\\"));

			name = GetShowName(_vCardFilter[i]);

			try{
				MakeValidPathName(name);
			}catch(exception &e){
				MessageBoxA(_hWnd, e.what(), "C++ Exception", MB_ICONERROR);
			}

			tmp += name + TEXT(".vcf");

			WriteVcfFile(tmp, _vCardFilter[i].getdata());
		}
	}
	else
	{
		int index = _vCardList.GetCurSel();
		if (index == LB_ERR || index >= (int)_vCardFilter.size())
			return ;

		path = GetShowName(_vCardFilter[index]);
		path.push_back(0);

		try{
			MakeValidPathName(path);
		}catch(exception &e){
			MessageBoxA(_hWnd, e.what(), "C++ Exception", MB_ICONERROR);
		}

		path = SelectFile(path);

		if (!path.empty()){
			WriteVcfFile(path, _vCardFilter[index].getdata());
		}
	}
}
//========================================================================
bool vCardWnd::WriteVcfFile(const TString &path, const string &buf)
{
	File file;
	if (!file.Open(path, File::write))
		return false;

	return file.Write((LPVOID)buf.c_str(), buf.length()) != 0;

	HANDLE hFile = INVALID_HANDLE_VALUE;
	bool ret = false;
	try
	{
#ifdef UNICODE
		hFile = CreateFileA(wtoa(path).c_str(), GENERIC_WRITE,  0,\
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
		hFile = CreateFileA(path.c_str(), GENERIC_WRITE,  0,\
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		DWORD dwWritten;
		ret = WriteFile(hFile, buf.c_str(), buf.length(), &dwWritten, NULL) ? true : false;

		CloseHandle(hFile);
	}
	catch(SException &e)
	{
		MessageBoxA(_hWnd, e.what(), "Exception", MB_ICONERROR);
	}

	return ret;
}
//========================================================================
TString vCardWnd::SelectFile(const TString &strDefName)
{
	OPENFILENAME ofn = {0};

	TCHAR filePath[MAX_PATH] = {0};

	lstrcpy(filePath, strDefName.c_str());

	ofn.lStructSize	= sizeof(OPENFILENAME);
	ofn.Flags		= OFN_OVERWRITEPROMPT | OFN_EXPLORER;
	ofn.hInstance	= getHinst();
	ofn.hwndOwner	= getSelf();
	ofn.lpstrFile	= filePath;
	ofn.nMaxFile	= MAX_PATH;
	ofn.lpstrTitle	= TEXT("保存vCard文件");
	ofn.lpstrFilter = TEXT("vCard文件(*.vcf)\0*.vcf\0\0");
	ofn.lpstrDefExt = TEXT("vcf");

	if (!GetSaveFileName(&ofn))
	{
		if (CommDlgExtendedError() != 0){
			msgBox(TEXT("保存文件失败"), _strCaption, MB_ICONERROR);
		}
		return TEXT("");
	}

	return filePath;
}
//========================================================================
TString vCardWnd::SelectFolder()
{
	BROWSEINFO bi = {0};       
	TCHAR pFileName[MAX_PATH];       
	ITEMIDLIST *pidl = {0};       

	bi.hwndOwner		 =   _hWnd;       
	bi.lpszTitle		 =   TEXT("请选择要保存到的文件夹");       
	bi.ulFlags			 =   BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_NEWDIALOGSTYLE;       

	pidl = ::SHBrowseForFolder(&bi);
	if (pidl == NULL)
		return TEXT("");

	if (!::SHGetPathFromIDList(pidl, pFileName))
		return TEXT("");

	return pFileName;
}
//========================================================================
void vCardWnd::PopupRKeyMenu()
{
	HMENU hMenu = 0;
	HMENU hMenuTP = 0;

	if (_vCardList.GetCount() < 1)
		return ;

	hMenu = LoadMenu(IDR_MENU_RKEY);
	if (!hMenu)	return ;

	hMenuTP = ::GetSubMenu(hMenu, 0);
	if (!hMenuTP)	return ;

	if (_vCardList.GetCurSel() == LB_ERR)
	{
		::EnableMenuItem(hMenuTP, IDM_SAVE_CUR, MF_BYCOMMAND | MF_DISABLED);
	}

	POINT pt;
	::GetCursorPos(&pt);
	::TrackPopupMenuEx(hMenuTP, TPM_BOTTOMALIGN, pt.x, pt.y, _hWnd, 0);

	::DestroyMenu(hMenu);

}
//========================================================================
TString vCardWnd::GetShowName(const vCard &vc)
{
	TString name;
	if (vc.name().empty())
	{
		if (vc.formattedname().empty())
		{
			if(vc.nickname().empty())
			{
				if (vc.tel().empty())
					name = TEXT("未知");
				else
					name = vc.tel();
			}
			else
			{
				name = vc.nickname();
			}
		}
		else
		{
			name = vc.formattedname();
		}
	}
	else
	{
		name = vc.name();
	}

	return name;
}
//========================================================================
void vCardWnd::MakeValidPathName(TString &what)
{
	//去掉空格
	what.trim();

	while (1)
	{
		int pos = what.find(TEXT(";"));
		if (pos == TString::npos)
			break;
		what.replace(pos, 1, TEXT(" "));
	}
	while (1)
	{
		int pos = what.find(TEXT("\r\n"));
		if (pos == TString::npos)
			break;
		what.replace(pos, 2, TEXT(" "));
	}
}
//========================================================================
DWORD WINAPI vCardWnd::HandleFilesThread(LPVOID lParam)
{
	vCardWnd *pWnd = (vCardWnd*)lParam;

	pWnd->HandleFile(pWnd->_pszFile);

	return 1;
}
//========================================================================
void vCardWnd::HandleFile(const TCHAR *filePaths)
{
	const TCHAR *p = filePaths;
	TCHAR szFile[MAX_PATH];

	if (!p){
		return ;
	}

	while(*p++)
		;

	if (*p == NULL) //只选了一个文件
	{
		TCHAR *ext = PathFindExtension(filePaths);
		if (ext)
		{
			if(lstrcmpi(ext, TEXT(".nbu")) == 0){
				if (!OpenNbuFile(filePaths)){
					msgBox(TEXT("打开文件失败了！"), _strCaption, MB_ICONERROR);
				}
			}else if(lstrcmpi(ext, TEXT(".vcf")) == 0){
				if (!OpenVcfFile(filePaths)){
					msgBox(TEXT("打开文件失败了！"), _strCaption, MB_ICONERROR);
				}
			}else{
				msgBox(TEXT("您只能选择*.vcf、*.nbu文件！"), _strCaption, MB_ICONINFORMATION);
			}
		}
	}
	else	//选择了多个文件
	{
		int nErrFiles = 0;
		while (*p)
		{
			lstrcpy(szFile, filePaths);
			lstrcat(szFile, TEXT("\\"));
			lstrcat(szFile, p);

			TCHAR *ext = PathFindExtension(szFile);
			if (ext)
			{
				TString info;
				info.format(TEXT("正在读取 %s ..."), szFile);
				ShowInfo(info);

				if(lstrcmpi(ext, TEXT(".nbu")) == 0){
					if (!OpenNbuFile(szFile))
					{
						TString info = szFile;
						info += TEXT(" 读取失败");
						ShowInfo(info);

						nErrFiles ++;
					}
				}else if(lstrcmpi(ext, TEXT(".vcf")) == 0){
					if (!OpenVcfFile(szFile))
					{
						TString info = szFile;
						info += TEXT(" 读取失败");
						ShowInfo(info);

						nErrFiles++;
					}
				}else{
					ShowInfo(TEXT("您只能选择*.vcf、*.nbu文件！"));
				}
			}

			while (*p++)
				;
		}

		ShowInfo(TEXT(""));

		if (nErrFiles > 0)
		{
			TString strInfo;
			strInfo.format(TEXT("共%d个文件读取失败！"), nErrFiles);
			msgBox(strInfo, _strCaption, MB_ICONERROR);
		}
	}

	if (_vCardList.GetCurSel() == LB_ERR && _vCardFilter.size()>0)
	{
		_vCardList.SetCurSel(0);
		sendMsg(WM_COMMAND, MAKEWPARAM(0, LBN_SELCHANGE), (LPARAM)_vCardList.getSelf());
	}
}
//========================================================================
void vCardWnd::ShowInfo(const TString &infoStr)
{
	_uTimeElapsed = 0;

	KillTimer(TIMER_INFO_ID);

	_status.setText(0, infoStr);

	SetTimer(TIMER_INFO_ID, 1000);
}
//========================================================================
void vCardWnd::OpenFromCmd(LPCTSTR lpCmd)
{
	TCHAR filePath[MAX_PATH] = {0};

	int i = 0;
	if (lstrcmp(lpCmd, TEXT(""))!=0)
	{
		//去掉空格和引号，如果存在的话
		while (*lpCmd == ' ' || *lpCmd == '\"')
			lpCmd++;

		while (*lpCmd && *lpCmd!='\"') //参数里有引号时要去掉
			filePath[i++] = *lpCmd++;

		filePath[i] = '\0';
	}

	TCHAR *ext = PathFindExtension(filePath);
	if (StrCmpI(ext, TEXT(".nbu")) == 0){
		OpenNbuFile(filePath);
	}else if(StrCmpI(ext, TEXT(".vcf")) == 0){
		OpenVcfFile(filePath);
	}

	if (_vCardFilter.size()>0)
	{
		_vCardList.SetCurSel(0);
		sendMsg(WM_COMMAND, MAKEWPARAM(0, LBN_SELCHANGE), (LPARAM)_vCardList.getSelf());
	}
}
//========================================================================
void vCardWnd::RegisteFileType(bool fRegister /* = true */)
{
	if (IsUACEnabled())
	{
		if (!IsProcessRunAsAdmin())
		{
			ShowInfo(TEXT("操作需管理员权限"));

			msgBox(TEXT("请您重启程序并以管理员权限运行程序，然后再操作。\r\n")
				TEXT("提示：鼠标右键程序选择“以管理员权限运行”即可。"),
				TEXT("操作需管理员权限"),
				MB_ICONINFORMATION);

			return ;
		}
	}

	if (fRegister)	/*注册*/
	{
		TCHAR szExePath[MAX_PATH];

		if (GetModuleFileName(getHinst(), szExePath, MAX_PATH) == 0)
		{
			ShowInfo(TEXT("获取程序路径时出错"));
			msgBox(TEXT("注册文件类型失败了！"), TEXT("错误"), MB_ICONERROR);
		}
		else
		{
			if(!RegHelper::RegisterFileType(TEXT(".nbu"), TEXT("Nokia_NBU_File"), szExePath))
			{
				ShowInfo(TEXT("注册NBU文件类型失败"));
				msgBox(TEXT("注册NBU文件类型失败了！"), TEXT("错误"), MB_ICONERROR);
			}
			else
			{
				ShowInfo(TEXT("成功注册NBU文件类型"));
			}

			if(!RegHelper::RegisterFileType(TEXT(".vcf"), TEXT("vCard_File"), szExePath))
			{
				ShowInfo(TEXT("注册VCF文件类型失败"));
				msgBox(TEXT("注册VCF文件类型失败！"), TEXT("错误"), MB_ICONERROR);
			}
			else
			{
				ShowInfo(TEXT("成功注册VCF文件类型"));
			}
		}
	}

	else			/*取消注册*/
	{
		if (!RegHelper::UnRegisterFileType(TEXT(".nbu"), TEXT("Nokia_NBU_File"))){
			msgBox(TEXT("移除注册NBU文件类型失败！"), TEXT("错误"), MB_ICONERROR);
		}else{
			ShowInfo(TEXT("移除注册NBU文件类型成功"));
		}

		if (!RegHelper::UnRegisterFileType(TEXT(".vcf"), TEXT("vCard_File"))){
			msgBox(TEXT("移除注册VCF文件类型失败！"), TEXT("错误"), MB_ICONERROR);
		}else{
			ShowInfo(TEXT("移除注册VCF文件类型成功"));
		}
	}
}

void vCardWnd::FiltervCard()
{
	TString key = _edFilter.getText();

	key.trim();

	vector<vCard>().swap(_vCardFilter);

	if (key.empty())
	{
		_vCardFilter = _vCard;
	}
	else
	{
		for(size_t i=0; i<_vCard.size(); i++)
		{
			if (vCardContain(_vCard[i], key)){
				_vCardFilter.push_back(_vCard[i]);
			}
		}
	}

	_vCardList.ResetContent();
	ClearRight();

	for(size_t i=0; i<_vCardFilter.size(); i++){
		_vCardList.AddString(GetShowName(_vCardFilter[i]));
	}

	Rect rect;
	_sInfo.GetWindowRect(&rect);
	ScreenToClient(&rect);

	InvalidateRect(&rect, TRUE);

	TString str;
	str.format(TEXT("( %d/%d )"), _vCardFilter.size(), _vCard.size());
	_sInfo.setText(str);
}

bool vCardWnd::vCardContain(const vCard &vc, TString &key)
{
	if (vc.name().find(key, 0, false) != TString::npos)
		return true;

	if (vc.formattedname().find(key, 0, false) != TString::npos )
		return true;

	if (vc.tel().find(key) != TString::npos )
		return true;

	if (vc.mail().find(key, 0, false) != TString::npos )
		return true;

	if (vc.nickname().find(key, 0, false) != TString::npos )
		return true;

	if (vc.title().find(key, 0, false) != TString::npos )
		return true;

	if (vc.url().find(key, 0, false) != TString::npos )
		return true;

	if (vc.birthday().find(key, 0, false) != TString::npos )
		return true;

	if (vc.address().find(key, 0, false) != TString::npos )
		return true;

	if (vc.organization().find(key, 0, false) != TString::npos )
		return true;

	if (vc.note().find(key, 0, false) != TString::npos )
		return true;

	//只对姓名特殊处理
	TString temp = vc.name();

	temp.Replace(TEXT(";"), TEXT(" "));
	if (temp.find(key, 0, false) != TString::npos)
		return true;

	temp.Replace(TEXT(" "), TEXT(""));
	key.Replace(TEXT(" "), TEXT(""));
	if (temp.find(key, 0, false) != TString::npos)
		return true;

	return false;
}

void vCardWnd::ClearRight()
{
	_lnameEdit.clear();
	_fnameEdit.clear();
	_fnEdit.clear();
	_telEdit.clear();
	_noteEdit.clear();
	_bdEdit.clear();
	_addrEdit.clear();
	_mailEdit.clear();
	_urlEdit.clear();
	_titleEdit.clear();
	_orgEdit.clear();
	_nnEdit.clear();
}