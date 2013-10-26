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

//========================================================================

#ifndef REG_HELPER_H
#define REG_HELPER_H

namespace RegHelper
{
	/*ע���׺ΪlpExt���ļ�����
	 *lpDesΪ��lpExt��˵��
	 *exePathΪִ�и�lpExt�ļ��ĳ���·��
	*/
	bool RegisterFileType(LPTSTR lpExt, LPTSTR lpDes, LPTSTR exePath);

	/*ȡ��ע��lpExt�ļ�����*/
	bool UnRegisterFileType(LPTSTR lpExt, LPTSTR lpDes);

	//��hKeyRoot�´����Ӽ�lpSubKey������������ΪlpValue������ΪlpData
	bool CreateKey(HKEY hKeyRoot, LPTSTR lpSubKey, LPTSTR lpValue, LPTSTR lpData);

	//ɾ��hKeyRoot�µ��Ӽ�lpSubKey���������Ӽ�
	bool DeleteKey(HKEY hKeyRoot, LPTSTR lpSubKey);

	bool DeleteKeyValue(HKEY hKeyRoot, LPTSTR lpSubKey, LPTSTR lpValue);

	bool IsKeyExists(HKEY hKeyRoot, LPTSTR lpValueName);
}
#endif REG_HELPER_H