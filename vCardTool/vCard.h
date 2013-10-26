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

#include "Tim\TString.h"
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

	//��0
	void clear();
	void analyse();

	TString name() const {
#ifdef UNICODE
		return atow(_name).c_str();
#else
		return _name;
#endif
	}
	TString formattedname() const{
#ifdef UNICODE
		return atow(_fn).c_str();
#else
		return _fn;
#endif
	}
	TString tel() const{
#ifdef UNICODE
		return atow(_tel).c_str();
#else
		return _tel;
#endif
	}

	TString mail() const {
#ifdef UNICODE
		return atow(_mail).c_str();
#else
		return _mail;
#endif
	}

	TString nickname() const {
#ifdef UNICODE
		return atow(_nickname).c_str();
#else
		return _nickname;
#endif
	}

	TString title() const {
#ifdef UNICODE
		return atow(_title).c_str();
#else
		return _title;
#endif
	}

	TString url() const {
#ifdef UNICODE
		return atow(_url).c_str();
#else
		return _url;
#endif
	}

	TString birthday() const {
#ifdef UNICODE
		return atow(_bday).c_str();
#else
		return _bday;
#endif
	}
	TString organization() const {
#ifdef UNICODE
		return atow(_org).c_str();
#else
		return _org;
#endif
	}

	TString address() const {
#ifdef UNICODE
		return atow(_addr).c_str();
#else
		return _addr;
#endif
	}

	TString note() const {
#ifdef UNICODE
		return atow(_note).c_str();
#else
		return _note;
#endif
	}

	BYTE* getPhotoData() const {	return _photoData;	}

	int Base64Encode(const BYTE *pSrc, int nSrcLen, BYTE *pDest);
	int Base64Decode(const BYTE *pSrc, int nSrcLen, BYTE *pDest);

	//Quoted-Printable����
	//��pDstΪNULLʱ���������С
	int QPDecode(const char* pSrc, int nSrcLen, unsigned char* pDst);

	//Quoted-Printable����
	int QPEncode(const unsigned char* pSrc, int nSrcLen, char* pDst, int nMaxLineLen);


protected:
	void BreakToLine();

	//�ж��Ƿ���ĳ����ǩ��ͷ
	bool istag(const char *pSrc, const char *tagName, int nSrcLen);
	
	//��ȡpSrc�е�ֵ��valueToStore
	void readtag(const char *pSrc, int len, string &valueToStore);

	void RemoveSemicolon(string &src);	//�Ƴ��������е�Ӣ�ķֺ�
	void RemoveEndChar(string &src);	//�Ƴ��������е�'\0'

	enum encoding{ ec_none, ec_qp, ec_base64, ec_8bit};
	enum charset{ cs_def, cs_utf8 };

	//����pSrc�е�ENCODING���ݣ��������
	encoding GetEncoding(const char *pSrc, int len);
	//CHARSET
	charset GetCharset(const char *pSrc, int len);
	//ȡ��':'�������
	string GetValue(const char *pSrc, int nSrcLen);

	//utf-8����ʱתΪansi
	string utf8toa(const char *str);

	void readphoto(const char *pSrc, int len);

private:
//	string _ver;		//�汾(VERSION)
	string _name;		//����(N)
	string _fn;			//��ʽ����(FN; formtted name)
	string _nickname;	//�ǳ�(NICKNAME)
	string _tel;		//����(TEL)
	string _note;		//ע��(NOTE)
	string _mail;		//�����ʼ�(EMAIL)
	string _title;		//ְ��(TITLE)
	string _url;		//��ַ(URL)
	string _bday;		//����(BDAY)
	string _org;		//��֯����(ORG)
	string _addr;		//��ַ(ADR)

	string _data;
	vector<string> _vlines;

	BYTE *_photoData;
};
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////