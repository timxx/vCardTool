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
#pragma once
//////////////////////////////////////////////////////////////////////////
#ifndef _VCARD_WND_H_
#define _VCARD_WND_H_
//////////////////////////////////////////////////////////////////////////
#include <vector>
#include "Tim\Window.h"
#include "resource.h"
#include "vCard.h"
#include "Tim\CommonCtrls.h"
//////////////////////////////////////////////////////////////////////////
//========================================================================
#define DEF_CXWND		480	//窗口默认大小
#define DEF_CYWND		500

#define EDIT_WIDTH		100	//编辑框宽\高
#define EDIT_HEIGHT		20

#define EDIT_SPACE		10	//编辑框之间的空隙

#define TIMER_INFO_ID	1
//========================================================================
using namespace Tim;
using namespace std;
//========================================================================
class vCardWnd : public Window
{
public:
	vCardWnd():_hFontChild(NULL), _strCaption(TEXT("名片小助手")),
		_hbmp(NULL), _hdcMem(NULL),_fDrew(false),
		_hThread(INVALID_HANDLE_VALUE), _pszFile(0),
		_uTimeElapsed(0)
	{}

	~vCardWnd()
	{}

	void init(HINSTANCE hinst, HWND hwndParent);
	void OpenFromCmd(LPCTSTR lpCmd);

	static TString getClassName() {
		return _clsName;
	}
private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK runProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnSize(int type, int nWidth, int nHeigth);
	void OnCommand(int id, HWND hwndCtl, UINT uNotifyCode);
	void OnPaint();
	void OnNotify(int id, NMHDR *pnmh);
	void OnDropFiles(HDROP hDrop);
	void OnTimer(UINT_PTR id);

	void doOpen();
	void doLBSelChange();

	//在状态栏上显示信息
	void ShowInfo(const TString &infoStr);

	//读取一个NBU文件
	bool OpenNbuFile(const TString &filePath);
	//解析NBU中的vCard内容
	//并显示到list box
	void AnalyseNbu(const char *buf, DWORD dwSize);

	bool IsvCardBegin(const char *buf);	//BEGIN:VCARD
	bool IsvCardEnd(const char *buf);	//END:VCARD

	bool OpenVcfFile(const TString &filePath);	//读取vCard文件
	void SaveVcfFile(bool fAll = false);		//保存vCard文件
	bool WriteVcfFile(const TString &path, const string &buf);		//写vCard文件

	TString SelectFolder();	//选择一个文件夹
	TString SelectFile(const TString &strDefName);	//选择个保存的文件路径

	static TString GetShowName(const vCard &vc);		//由vCard返回一个非空名字
	void MakeValidPathName(TString &what);		//去除路径非法字符

	void PopupRKeyMenu();

	static DWORD WINAPI HandleFilesThread(LPVOID lParam);

	void HandleFile(const TCHAR *filePaths);

	void RegisteFileType(bool fRegister = true);

	void FiltervCard();
	bool vCardContain(const vCard &vc, TString &key);
	void ClearRight();	//清空右边显示的
private:
	static TString _clsName;
	TString _strCaption;
	HFONT _hFontChild;

	vector<vCard> _vCard;
	vector<vCard> _vCardFilter;

	ListBox _vCardList;
	StatusBar _status;

	Edit _lnameEdit;	//姓
	Edit _fnameEdit;	//名
	Edit _fnEdit;		//正式姓名
	Edit _nnEdit;		//昵称
	Edit _telEdit;		//号码
	Edit _noteEdit;		//注释
	Edit _mailEdit;		//电子邮件
	Edit _titleEdit;	//职务
	Edit _urlEdit;		//网址
	Edit _bdEdit;		//生日
	Edit _orgEdit;		//公司
	Edit _addrEdit;		//通讯地址

	Edit _edFilter;		//筛选编辑框
	Static _sInfo;		//显示名片条数

	HDC _hdcMem;		//
	HBITMAP _hbmp;		//
	bool _fDrew;		//标记是否已经画过

	HANDLE _hThread;
	CRITICAL_SECTION _cs;

	TCHAR *_pszFile;

	UINT _uTimeElapsed;
};
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////