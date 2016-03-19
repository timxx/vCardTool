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


#include "common.h"

using namespace Tim;
using std::max;

HBITMAP GradienBitmap(HWND hWnd, COLORREF cr1, COLORREF cr2)
{
	Rect rcClient;
	::GetClientRect(hWnd, &rcClient);

	HDC hdc = GetDC(hWnd);

	int nRed	= GetRValue(cr2) - GetRValue(cr1),
		nGreen	= GetGValue(cr2) - GetGValue(cr1),
		nBlue	= GetBValue(cr2) - GetBValue(cr1);

	float fRefStep = 0.0,
		fGreenStep = 0.0,
		fBlueStep  = 0.0;

	//取最大的色差
	int nSteps = max(abs(nRed), max(abs(nGreen), abs(nBlue)));

	//确定每一颜色填充多大的矩形区域
	float fStep = (float)(rcClient.Width() + 5)/(float)nSteps;

	//设置每一颜色填充的步数
	fRefStep	= nRed/(float)nSteps;
	fGreenStep	= nGreen/(float)nSteps;
	fBlueStep	= nBlue/(float)nSteps;

	nRed	= GetRValue(cr1);
	nGreen	= GetGValue(cr1);
	nBlue	= GetBValue(cr1);

	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hbmp = CreateCompatibleBitmap(hdc, rcClient.Width(), rcClient.Height());

	HGDIOBJ hOldObj = SelectObject(hdcMem, hbmp);

	Rect rcFill;

	for(int nOnBand=0; nOnBand < nSteps; nOnBand++)
	{
		SetRect(&rcFill, int(nOnBand * fStep),
			0, int((nOnBand+1)*fStep), rcClient.bottom + 1);

		HBRUSH hbr = CreateSolidBrush(
			RGB( nRed + fRefStep * nOnBand,
			nGreen + fGreenStep * nOnBand,
			nBlue + fBlueStep * nOnBand )
			);

		HGDIOBJ hOldBrush = SelectObject(hdcMem, hbr);

		FillRect(hdcMem, &rcFill, hbr);

		SelectObject(hdcMem, hOldBrush);
		DeleteObject(hbr);

	}

	SelectObject(hdcMem, hOldObj);
	DeleteDC(hdcMem);
	ReleaseDC(hWnd, hdc);

	return hbmp;
}

void DrawBitmap(const HBITMAP &hBitmap, HDC hdc, const Rect &rect)
{
	if (!hBitmap || !hdc){
		return;
	}

	HDC hdcMem = CreateCompatibleDC(hdc);
	HGDIOBJ hOldObj = SelectObject(hdcMem, hBitmap);

	BitBlt(hdc, rect.left, rect.top, rect.Width(), rect.Height(), hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, hOldObj);
	DeleteDC(hdcMem);
}

bool IsVistaOrLater()
{
	OSVERSIONINFO osvi = {0};
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (!GetVersionEx(&osvi))//获取失败当XP好了
		return false;

	return osvi.dwMajorVersion >= 6;
}


bool IsUACEnabled()
{
	if (!IsVistaOrLater())
		return false;

	bool fEnabledUAC = false;

	HKEY hKEY = NULL;

	DWORD dwType = REG_DWORD;
	DWORD dwEnableLUA = 0;
	DWORD dwSize = sizeof(DWORD);

	LONG status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\"),
		0, KEY_READ, &hKEY);

	if (ERROR_SUCCESS == status)
	{
		status = RegQueryValueEx(hKEY, TEXT("EnableLUA"),
			NULL, &dwType, (BYTE*)&dwEnableLUA, &dwSize);

		if (ERROR_SUCCESS == status){
			fEnabledUAC = (dwEnableLUA == 1);
		}

		RegCloseKey(hKEY);
	}

	return fEnabledUAC;
}

bool IsProcessRunAsAdmin()
{
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;

	BOOL b = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&AdministratorsGroup);

	if(b)
	{
		CheckTokenMembership(NULL, AdministratorsGroup, &b);
		FreeSid(AdministratorsGroup);
	}

	return b == TRUE;
 }
