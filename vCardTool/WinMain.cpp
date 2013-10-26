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

//========================================================================================================
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
	"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
//========================================================================================================
//////////////////////////////////////////////////////////////////////////
#include "vCardWnd.h"
#include "Tim\SException.h"
//////////////////////////////////////////////////////////////////////////
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE, LPTSTR lpCmdLine, int nCmdShow)
{
	MSG msg = {0};

	CreateMutex(NULL, FALSE, TEXT("vCardToolMutex"));

	//�Ѿ�������һ��ʵ��
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		HWND hWnd = FindWindow(vCardWnd::getClassName(), NULL);

		//���������С��
		//�Ȼ�ԭ����
		if (IsIconic(hWnd)){
			SendMessage(hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		}
		//����ǰ����
		SetForegroundWindow(hWnd);

		if (*lpCmdLine)
		{
			COPYDATASTRUCT cds;
			RtlSecureZeroMemory(&cds, sizeof(COPYDATASTRUCT));

			cds.dwData = 0;
			cds.cbData = (lstrlen(lpCmdLine)+1)*sizeof(TCHAR);
			cds.lpData = lpCmdLine;

			SendMessage(hWnd, WM_COPYDATA, (WPARAM)hInst, (LPARAM)&cds);
		}

		return 0;
	}

	Tim::SException::install();

	vCardWnd vcWnd;

	try
	{
		vcWnd.init(hInst, NULL);

		if (*lpCmdLine){
			vcWnd.OpenFromCmd(lpCmdLine);
		}

		HACCEL hAcc = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR1));

		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (!TranslateAccelerator(vcWnd.getSelf(), hAcc, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	catch(SException &e){
			MessageBoxA(GetDesktopWindow(), e.what(), "Exception", MB_ICONERROR);
	}catch(std::exception &e){
		MessageBoxA(GetDesktopWindow(), e.what(), "C++ Standard Error", MB_ICONERROR);
	}catch(...){
		MessageBox(GetDesktopWindow(), TEXT("Unknown error"), TEXT("Error"), MB_ICONERROR);
	}

	return msg.wParam;
}