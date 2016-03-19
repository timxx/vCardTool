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

#include <windows.h>

#include "RegHelper.h"

//========================================================================

bool RegHelper::RegisterFileType(LPCTSTR lpExt, LPCTSTR lpDes, LPCTSTR exePath)
{
	if(!CreateKey(HKEY_CLASSES_ROOT, lpExt, NULL, lpDes))	return false;

	if(!CreateKey(HKEY_CLASSES_ROOT, lpDes, NULL, lpDes))	return false;

	TCHAR key[MAX_PATH];
	TCHAR value[MAX_PATH];

	wsprintf(key, TEXT("%s\\DefaultIcon"), lpDes);
	wsprintf(value, TEXT("\"%s\",0"), exePath);

	if (!CreateKey(HKEY_CLASSES_ROOT, key, NULL, value))	return false;
	
	wsprintf(key, TEXT("%s\\shell\\open\\command"), lpDes);
	//wsprintf不认%1的帐……
	wsprintf(value, TEXT("\"%s\" \"%%1"), exePath);
//	lstrcat(value, TEXT("%1\""));

	if (!CreateKey(HKEY_CLASSES_ROOT, key, NULL, value))	return false;

	return true;
}

//========================================================================

bool RegHelper::UnRegisterFileType(LPCTSTR lpExt, LPCTSTR lpDes)
{
	if (!DeleteKey(HKEY_CLASSES_ROOT, lpExt))	return false;

	return DeleteKey(HKEY_CLASSES_ROOT, lpDes);
}

bool RegHelper::CreateKey(HKEY hKeyRoot, LPCTSTR lpSubKey, LPCTSTR lpValue, LPCTSTR lpData)
{
	HKEY hKey;
	long lResult = ERROR_SUCCESS;

	lResult = ::RegCreateKey(hKeyRoot, lpSubKey, &hKey);

	if (lResult != ERROR_SUCCESS)	return false;

	lResult = ::RegSetValueEx(hKey, lpValue, 0, REG_SZ, (const BYTE*)lpData, (lstrlen(lpData)+1)*sizeof(TCHAR));

	if (lResult != ERROR_SUCCESS)
	{
		::RegCloseKey(hKey);
		return false;
	}

	::RegCloseKey(hKey);

	return true;
}

bool RegHelper::DeleteKey(HKEY hKeyRoot, LPCTSTR lpSubKey)
{
	HKEY hKey;
    long lResult;
    DWORD dwSize;
	TCHAR szName[MAX_PATH] ={0};
	TCHAR szSubKey[MAX_PATH] ={0};

    lResult = ::RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS)	return true;

    lResult = ::RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) 
    {
        if (lResult == ERROR_FILE_NOT_FOUND)	return true;

        else	return false;		//打开键值出错
    }

    dwSize = MAX_PATH;
    lResult = ::RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, NULL);

    if (lResult == ERROR_SUCCESS) 
    {
        do 
		{
			lstrcpy(szSubKey, lpSubKey);
			lstrcat(szSubKey, TEXT("\\"));
			lstrcat(szSubKey, szName);

			//递归调用删除下一层子键
            if (!DeleteKey(hKeyRoot, szSubKey))	break;

            dwSize = MAX_PATH;

            lResult = ::RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, NULL);

        } while (lResult == ERROR_SUCCESS);
    }

   ::RegCloseKey (hKey);

   lResult = ::RegDeleteKey(hKeyRoot, lpSubKey);

   if (lResult == ERROR_SUCCESS)	return true;

   return false;
}

bool RegHelper::DeleteKeyValue(HKEY hKeyRoot, LPCTSTR lpSubKey, LPCTSTR lpValue)
{

	HKEY hKey;
	LONG lResult;
	DWORD dwp = REG_OPENED_EXISTING_KEY;

	lResult = ::RegCreateKeyEx(hKeyRoot, lpSubKey, 0, NULL,\
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,  &hKey, &dwp);

	if (lResult!=ERROR_SUCCESS)
		return false;

	if (!IsKeyExists(hKey, lpValue))	return true;

	lResult = ::RegDeleteValue(hKey, lpValue);

	if (lResult!=ERROR_SUCCESS)
	{
		::RegCloseKey(hKey);
		return false;
	}

	::RegCloseKey(hKey);
	return true;
}

bool RegHelper::IsKeyExists(HKEY hKeyRoot, LPCTSTR lpValueName)
{
	LONG lResult = ::RegQueryValueEx(hKeyRoot, lpValueName, 0, NULL, NULL, NULL);

	if (lResult != ERROR_SUCCESS)
	{
		if (lResult == ERROR_FILE_NOT_FOUND)
			return false;
	}

	return true;
}
