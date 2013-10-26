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
#define DEF_CXWND		480	//����Ĭ�ϴ�С
#define DEF_CYWND		500

#define EDIT_WIDTH		100	//�༭���\��
#define EDIT_HEIGHT		20

#define EDIT_SPACE		10	//�༭��֮��Ŀ�϶

#define TIMER_INFO_ID	1
//========================================================================
using namespace Tim;
using namespace std;
//========================================================================
class vCardWnd : public Window
{
public:
	vCardWnd():_hFontChild(NULL), _strCaption(TEXT("��ƬС����")),
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

	//��״̬������ʾ��Ϣ
	void ShowInfo(const TString &infoStr);

	//��ȡһ��NBU�ļ�
	bool OpenNbuFile(const TString &filePath);
	//����NBU�е�vCard����
	//����ʾ��list box
	void AnalyseNbu(const char *buf, DWORD dwSize);

	bool IsvCardBegin(const char *buf);	//BEGIN:VCARD
	bool IsvCardEnd(const char *buf);	//END:VCARD

	bool OpenVcfFile(const TString &filePath);	//��ȡvCard�ļ�
	void SaveVcfFile(bool fAll = false);		//����vCard�ļ�
	bool WriteVcfFile(const TString &path, const string &buf);		//дvCard�ļ�

	TString SelectFolder();	//ѡ��һ���ļ���
	TString SelectFile(const TString &strDefName);	//ѡ���������ļ�·��

	static TString GetShowName(const vCard &vc);		//��vCard����һ���ǿ�����
	void MakeValidPathName(TString &what);		//ȥ��·���Ƿ��ַ�

	void PopupRKeyMenu();

	static DWORD WINAPI HandleFilesThread(LPVOID lParam);

	void HandleFile(const TCHAR *filePaths);

	void RegisteFileType(bool fRegister = true);

	void FiltervCard();
	bool vCardContain(const vCard &vc, TString &key);
	void ClearRight();	//����ұ���ʾ��
private:
	static TString _clsName;
	TString _strCaption;
	HFONT _hFontChild;

	vector<vCard> _vCard;
	vector<vCard> _vCardFilter;

	ListBox _vCardList;
	StatusBar _status;

	Edit _lnameEdit;	//��
	Edit _fnameEdit;	//��
	Edit _fnEdit;		//��ʽ����
	Edit _nnEdit;		//�ǳ�
	Edit _telEdit;		//����
	Edit _noteEdit;		//ע��
	Edit _mailEdit;		//�����ʼ�
	Edit _titleEdit;	//ְ��
	Edit _urlEdit;		//��ַ
	Edit _bdEdit;		//����
	Edit _orgEdit;		//��˾
	Edit _addrEdit;		//ͨѶ��ַ

	Edit _edFilter;		//ɸѡ�༭��
	Static _sInfo;		//��ʾ��Ƭ����

	HDC _hdcMem;		//
	HBITMAP _hbmp;		//
	bool _fDrew;		//����Ƿ��Ѿ�����

	HANDLE _hThread;
	CRITICAL_SECTION _cs;

	TCHAR *_pszFile;

	UINT _uTimeElapsed;
};
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////