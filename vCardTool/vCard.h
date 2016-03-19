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
#ifndef _VCARD_H_
#define _VCARD_H_
//////////////////////////////////////////////////////////////////////////
#include <vector>

#include "Tim/TString.h"
//////////////////////////////////////////////////////////////////////////
using namespace std;
using namespace Tim;
//////////////////////////////////////////////////////////////////////////
//========================================================================
class vCard
{
public:
	vCard():_photoData(0){}
	vCard(const string &data)
		:_data(data), _photoData(0)
	{
		BreakToLine();
		analyse();
	}

	void setdata(const string &data){
		_data = data;
		BreakToLine();
		analyse();
	}

	string getdata() const{
		return _data;
	}

	//清0
	void clear();
	void analyse();

	TString name() const {
#ifdef UNICODE
		return atow(_name).c_str();
#else
		return _name.c_str();
#endif
	}
	TString formattedname() const{
#ifdef UNICODE
		return atow(_fn).c_str();
#else
		return _fn.c_str();
#endif
	}
	TString tel() const{
#ifdef UNICODE
		return atow(_tel).c_str();
#else
		return _tel.c_str();
#endif
	}

	TString mail() const {
#ifdef UNICODE
		return atow(_mail).c_str();
#else
		return _mail.c_str();
#endif
	}

	TString nickname() const {
#ifdef UNICODE
		return atow(_nickname).c_str();
#else
		return _nickname.c_str();
#endif
	}

	TString title() const {
#ifdef UNICODE
		return atow(_title).c_str();
#else
		return _title.c_str();
#endif
	}

	TString url() const {
#ifdef UNICODE
		return atow(_url).c_str();
#else
		return _url.c_str();
#endif
	}

	TString birthday() const {
#ifdef UNICODE
		return atow(_bday).c_str();
#else
		return _bday.c_str();
#endif
	}
	TString organization() const {
#ifdef UNICODE
		return atow(_org).c_str();
#else
		return _org.c_str();
#endif
	}

	TString address() const {
#ifdef UNICODE
		return atow(_addr).c_str();
#else
		return _addr.c_str();
#endif
	}

	TString note() const {
#ifdef UNICODE
		return atow(_note).c_str();
#else
		return _note.c_str();
#endif
	}

	BYTE* getPhotoData() const {	return _photoData;	}

	int Base64Encode(const BYTE *pSrc, int nSrcLen, BYTE *pDest);
	int Base64Decode(const BYTE *pSrc, int nSrcLen, BYTE *pDest);

	//Quoted-Printable解码
	//当pDst为NULL时返回所需大小
	int QPDecode(const char* pSrc, int nSrcLen, unsigned char* pDst);

	//Quoted-Printable编码
	int QPEncode(const unsigned char* pSrc, int nSrcLen, char* pDst, int nMaxLineLen);


protected:
	void BreakToLine();

	//判断是否是某个标签的头
	bool istag(const char *pSrc, const char *tagName, int nSrcLen);
	
	//读取pSrc行的值到valueToStore
	void readtag(const char *pSrc, int len, string &valueToStore);

	void RemoveSemicolon(string &src);	//移除后面所有的英文分号
	void RemoveEndChar(string &src);	//移除后面所有的'\0'

	enum encoding{ ec_none, ec_qp, ec_base64, ec_8bit};
	enum charset{ cs_def, cs_utf8 };

	//返回pSrc行的ENCODING内容，如果存在
	encoding GetEncoding(const char *pSrc, int len);
	//CHARSET
	charset GetCharset(const char *pSrc, int len);
	//取自':'后的内容
	string GetValue(const char *pSrc, int nSrcLen);

	//utf-8编码时转为ansi
	string utf8toa(const char *str);

	void readphoto(const char *pSrc, int len);

private:
//	string _ver;		//版本(VERSION)
	string _name;		//名字(N)
	string _fn;			//正式名称(FN; formtted name)
	string _nickname;	//昵称(NICKNAME)
	string _tel;		//号码(TEL)
	string _note;		//注释(NOTE)
	string _mail;		//电子邮件(EMAIL)
	string _title;		//职务(TITLE)
	string _url;		//网址(URL)
	string _bday;		//生日(BDAY)
	string _org;		//组织名称(ORG)
	string _addr;		//地址(ADR)

	string _data;
	vector<string> _vlines;

	BYTE *_photoData;
};
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////
