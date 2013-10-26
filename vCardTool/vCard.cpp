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
#define _CRT_SECURE_NO_WARNINGS
//========================================================================
#include <Windows.h>
#include <shlwapi.h>

#include "vCard.h"
//========================================================================
void vCard::clear()
{
//	_ver.clear();
	_nickname.clear();
	_name.clear();
	_fn.clear();
	_title.clear();
	_tel.clear();
	_note.clear();
	_mail.clear();
	_url.clear();
	_bday.clear();
	_org.clear();
	_addr.clear();
}
//========================================================================
void vCard::analyse()
{
	clear();

	if (_vlines.size() < 1)
		return ;

	for (size_t i=0; i<_vlines.size(); i++)
	{
		const char *p = _vlines[i].c_str();
		int len = _vlines[i].length();

		if (istag(p, "BEGIN:VCARD", len))
			continue;
		else if (istag(p, "VERSION:", len))
			continue;
		else if (istag(p, "FN;", len) || istag(p, "FN:", len))
			readtag(p, len, _fn);
		else if (istag(p, "N;", len) || istag(p, "N:", len))
			readtag(p, len, _name);
		else if (istag(p, "TEL;", len) || istag(p, "TEL:", len))
			readtag(p, len, _tel);
		else if (istag(p, "NOTE:", len) || istag(p, "NOTE;", len))
			readtag(p, len, _note);
		else if (istag(p, "EMAIL;", len) || istag(p, "EMAIL:", len))
			readtag(p, len, _mail);
		else if (istag(p, "NICKNAME;", len) || istag(p, "NICKNAME:", len))
			readtag(p, len, _nickname);
		else if (istag(p, "TITLE;", len) || istag(p, "TITLE:", len))
			readtag(p, len, _title);
		else if (istag(p, "URL;", len) || istag(p, "URL:", len))
			readtag(p, len, _url);
		else if (istag(p, "BDAY;", len) || istag(p, "BDAY:", len))
			readtag(p, len, _bday);
		else if (istag(p, "ORG;", len) || istag(p, "ORG:", len))
			readtag(p, len, _org);
		else if(istag(p, "ADR;", len) || istag(p, "ADR:", len))
			readtag(p, len, _addr);
		else if(istag(p, "PHOTO;", len) || istag(p, "PHOTO:", len))
			readphoto(p, len);
	}

	RemoveSemicolon(_name);
	RemoveSemicolon(_fn);
	RemoveSemicolon(_nickname);
	RemoveSemicolon(_mail);
	RemoveSemicolon(_addr);
	RemoveSemicolon(_url);
	RemoveSemicolon(_bday);
	RemoveSemicolon(_note);
	RemoveSemicolon(_org);
	RemoveSemicolon(_title);
	RemoveSemicolon(_tel);
}
//========================================================================
vCard::encoding vCard::GetEncoding(const char *pSrc, int len)
{
	const char *p = pSrc;

	const char *q = StrStrIA(p, "ENCODING=");

	if (!q){
		return ec_none;
	}

	p += int(q - p) + 9;
	len -= int(q -q) + 9;

	string value;
	encoding ec = ec_none;

	while (len >= 0 && *p != ';' && *p != ':' && *p != '\r' && *p != '\n')
	{
		value.push_back(*p);
		p++;
		len--;
	}

	if (StrCmpIA(value.c_str(), "QUOTED-PRINTABLE")==0)
		ec = ec_qp;
	else if(StrCmpIA(value.c_str(), "8BIT")==0)
		ec = ec_8bit;
	else if(StrCmpIA(value.c_str(), "BASE64")==0)
		ec = ec_base64;

	return ec;
}
//========================================================================
vCard::charset vCard::GetCharset(const char *pSrc, int len)
{
	int i = 0;
	charset cs = cs_def;
	const char *p = pSrc;

	const char *q = StrStrIA(p, "CHARSET=");

	if (!q){
		return cs_def;
	}

	p += int(q - p) + 8;
	len -= int(q -q) + 8;


	string value;

	while (len >= 0 && *p && *p != ';' && *p != ':' && *p != '\r' && *p != '\n')
	{
		value.push_back(*p);
		p++;
		len--;
	}

	if (StrCmpIA(value.c_str(), "UTF-8")==0)
		cs = cs_utf8;

	return cs;
}
//========================================================================
int vCard::Base64Encode(const BYTE *pSrc, int nSrcLen, BYTE *pDest)
{
	const BYTE map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	if (!pDest){
		return (nSrcLen + 2) / 3 * 4;
	}

	int k = nSrcLen - nSrcLen % 3;
	int i = 0, j = 0;
	for (i=0; i<k; i+=3)
	{
		pDest[j++] = map[(pSrc[i]>>2)&0x3F];
		pDest[j++] = map[((pSrc[i]<<4)+(pSrc[i+1]>>4))&0x3F];
		pDest[j++] = map[((pSrc[i+1]<<2)+(pSrc[i+2]>>6))&0x3F];
		pDest[j++] = map[(pSrc[i+2])&0x3F];
	}
	k = nSrcLen - k;

	if(1 == k)
	{
		pDest[j++] = map[(pSrc[i]>>2)&0x3F] ;
		pDest[j++] = map[(pSrc[i]<<4)&0x3F] ;
		pDest[j++] = pDest[j] = '=' ;
	}
	else if(2 == k)
	{
		pDest[j++] = map[(pSrc[i]>>2)&0x3F] ;
		pDest[j++] = map[((pSrc[i]<<4) + (pSrc[i+1]>>4))&0x3F] ;
		pDest[j++] = map[(pSrc[i+1]<<2)&0x3F] ;
		pDest[j] = '=' ;
	}

	return ++j;
}
//========================================================================
int vCard::Base64Decode(const BYTE *pSrc, int nSrcLen, BYTE *pDest)
{
	const BYTE map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	BYTE DecodeBase64Map[256] = {0} ;
	int i, j;

	 //忽略=号
	for(j = nSrcLen; j>0 && '=' == pSrc[j-1]; --j)
		;

	if (!pDest){
		return (j/4)*3 + (j%4 + 1)/2;
	}

	for(i=0; i<64; ++i)
		DecodeBase64Map[map[i]] = i;

	for(i=j=0; i<nSrcLen; i+=4)
	{
		pDest[j++] = DecodeBase64Map[pSrc[i  ]] << 2 | DecodeBase64Map[pSrc[i+1]] >> 4 ;
		pDest[j++] = DecodeBase64Map[pSrc[i+1]] << 4 | DecodeBase64Map[pSrc[i+2]] >> 2 ;
		pDest[j++] = DecodeBase64Map[pSrc[i+2]] << 6 | DecodeBase64Map[pSrc[i+3]] ;
	}

	return j;
}
//========================================================================
int vCard::QPEncode(const unsigned char* pSrc, int nSrcLen, char* pDst, int nMaxLineLen)
{
	int nDstLen = 0;        // 输出的字符计数
	int nLineLen = 0;       // 输出的行长度计数

	for (int i = 0; i < nSrcLen; i++, pSrc++)
	{
		// ASCII 33-60, 62-126原样输出，其余的需编码
		if ((*pSrc >= '!') && (*pSrc <= '~') && (*pSrc != '='))
		{
			if (pDst){
				*pDst++ = (char)*pSrc;
			}

			nDstLen++;
			nLineLen++;
		}
		else
		{
			if (pDst)
			{
				sprintf(pDst, "=%02X", *pSrc);
				pDst += 3;
			}
			nDstLen += 3;
			nLineLen += 3;
		}

		// 输出换行
		if (nLineLen >= nMaxLineLen - 3)
		{
			if (pDst)
			{
				sprintf(pDst, "=\r\n");
				pDst += 3;
			}
			nDstLen += 3;
			nLineLen = 0;
		}
	}

	if (pDst){
		*pDst = '\0';
	}

	return nDstLen;
}
//========================================================================
int vCard::QPDecode(const char* pSrc, int nSrcLen, unsigned char* pDst)
{
	int nDstLen = 0;        // 输出的字符计数
	int i = 0;

	while (i < nSrcLen)
	{
		if (strncmp(pSrc, "=\r\n", 3) == 0)        // 软回车，跳过
		{
			pSrc += 3;
			i += 3;
		}
		else
		{
			if (*pSrc == '=')        // 是编码字节
			{
				if (pDst)
				{
					//这函数总是有问题！！！不用了
					//sscanf_s(pSrc, "=%02X", pDst);

					pSrc++;	//跳过=
					char ch[3] = {0};
					size_t j = 0;

					ch[j++] = *pSrc++;
					ch[j] = *pSrc++;

					*pDst++ = 
						(ch[0] >= 'A' ? (ch[0] - 'A' + 10) : (ch[0] - '0'))*16 + 
						(ch[1] >= 'A' ? (ch[1] - 'A' + 10) : (ch[1] - '0'));

					i += 3;
				}
				else
				{
					pSrc += 3;
					i += 3;
				}
			}
			else        // 非编码字节
			{
				if (pDst){
					*pDst++ = (unsigned char)*pSrc++;
				}else{
					pSrc++;
				}

				i++;
			}

			nDstLen++;
		}
	}

	if (pDst){
		*pDst = '\0';
	}

	return nDstLen;
}
//========================================================================
string vCard::GetValue(const char *pSrc, int nSrcLen)
{
	const char *p = pSrc;

	while (nSrcLen >= 0 && *p != ':')
		p++, nSrcLen--;

	if (nSrcLen < 0)
		return "";

	p++, nSrcLen--;

	string value;

	while (nSrcLen >= 0)
	{
		value.push_back(*p);
		p++;
		nSrcLen--;
	}

	return value;
}
//========================================================================
string vCard::utf8toa(const char *str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, (char*)str, -1, 0, 0);
	wchar_t *wstr = new wchar_t[len+1];

	MultiByteToWideChar(CP_UTF8, 0, (char*)str, -1, wstr, len);

	len = WideCharToMultiByte(CP_ACP, 0, wstr, len, 0, 0, 0, 0);
	char *astr = new char[len+1];

	WideCharToMultiByte(CP_ACP, 0, wstr, -1, astr, len, NULL, NULL);

	string res = astr;

	delete [] wstr;
	delete [] astr;

	return res;
}
//========================================================================
bool vCard::istag(const char *pSrc, const char *tagName, int nSrcLen)
{
	const char *p = pSrc;
	const char *q = tagName;

	//不区分大小写
	while (nSrcLen >=0 && *q && (islower(*p) ? *p - 32 : *p) == *q)
		p++, q++, nSrcLen--;

	if (!*q){
		return true;
	}

	return false;
}
//========================================================================
void vCard::BreakToLine()
{
	if (_data.empty())
		return;

	const char *p = _data.c_str();
	int len = _data.length();

	while (len >= 0 && *p)
	{
		string line;

		while (len >= 0 && *p && *p != '\r' && *p != '\n')
		{
			line.push_back(*p);
			p++;
			len--;
		}

		if (!line.empty()){
			_vlines.push_back(line);
		}

		if (len < 0)
			break;
		while (len >= 0 && *p && (*p == '\r' || *p == '\n'))
			p++, len--;
		if (len < 0 || !*p)
			break;

		//接着上一行的
		if (*p == '=')
		{
			//Fixed: if line is empty, then pop_back would
			// throw a invalid string pos exception
			//Apr. 30, 2011
			if (line.empty())
				continue;

			line.pop_back();	//删除最后一个'='

			while (len >= 0 && *p && *p != '\r' && *p != '\n')
			{
				line.push_back(*p);
				p++;
				len--;
			}
			if (!line.empty())
			{
				//删掉最后一个'='
				if (line.at(line.length()-1) == '=')
					line.pop_back();

				_vlines.pop_back();
				_vlines.push_back(line);
			}
		}
		//p++, len--;
	}
}
//========================================================================
void vCard::RemoveSemicolon(string &src)
{
	if (src.empty())
		return ;

	RemoveEndChar(src);

	if (src.empty())
		return ;

	while (1)
	{
		int pos = src.rfind(";");
		if (pos == string::npos)
			break;
		else if(pos != src.length()-1)
			break;

		src.erase(pos);
	}

	src.push_back(0);
}
//========================================================================
void vCard::RemoveEndChar(string &src)
{
	while (src.length() > 0 && src.at(src.length()-1) == 0)
		src.pop_back();
}
//========================================================================
void vCard::readtag(const char *pSrc, int len, string &valueToStore)
{
	const char *p = pSrc;

	encoding ec = GetEncoding(p, len);
	charset cs = GetCharset(p, len);

	string value = GetValue(p, len);

	//空值，返回
	if (value.empty())
		return ;

	RemoveSemicolon(value);

	string decStr = value;
	char *tmpStr = NULL;

	if (ec == ec_qp)
	{
		int deslen = QPDecode(value.c_str(), value.length(), NULL);
		if (deslen <= 0)
			return ;

		tmpStr = new char[deslen+1];
		SecureZeroMemory(tmpStr, deslen+1);

		QPDecode(value.c_str(), value.length(), (unsigned char*)tmpStr);

		decStr = tmpStr;
	}
	else if(ec == ec_base64)
	{
		int deslen = Base64Decode((BYTE*)value.c_str(), value.length(), NULL);

		tmpStr = new char[deslen + 1];
		SecureZeroMemory(tmpStr, deslen+1);

		Base64Decode((BYTE*)value.c_str(), value.length(), (BYTE*)tmpStr);

		decStr = tmpStr;
	}

	if (!valueToStore.empty())	//之前已赋过值，加到末尾
	{
		if (cs == cs_utf8)
			valueToStore +=  utf8toa(decStr.c_str());
		else if (cs == cs_def)
			valueToStore += decStr;
		
		RemoveEndChar(valueToStore);

		valueToStore += ";";
	}
	else
	{
		if (cs == cs_utf8)
			valueToStore =  utf8toa(decStr.c_str());
		else/* if (cs == cs_def)*/
			valueToStore = decStr;

		RemoveEndChar(valueToStore);

		valueToStore += ";";
	}

	if (tmpStr){
		delete [] tmpStr;
	}
}
//========================================================================
void vCard::readphoto(const char *pSrc, int len)
{
	if (!pSrc || len == 0)
		return ;
}
//========================================================================