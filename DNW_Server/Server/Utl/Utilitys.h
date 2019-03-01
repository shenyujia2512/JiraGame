#pragma once
#include "utility.h"

//设置get set 属性宏
#define GS_Field(TYPE,VAR,FUNC) \
private:TYPE VAR; \
public:TYPE get##FUNC(){ return VAR; }\
public:void set##FUNC(TYPE _args){ VAR = _args; }

#define G_Field(TYPE,VAR,FUNC) \
private:TYPE VAR; \
public:TYPE get##FUNC(){ return VAR; }

#define SAFE_DELETE(p) if(p != nullptr){ delete p;p = nullptr;}
#define SAFE_DELETE_ARRAY(p) if(p != nullptr){ delete[] p;p = nullptr;}
#define Gen_DELETE(p) delete p;p = nullptr

//生成RandomID
string GenrateRandomID();

string int2String(int iValue);

int string2Int(string str);

// string的编码方式为utf8，则采用：
std::string wstring2utf8string(const std::wstring& str);

std::wstring utf8string2wstring(const std::string& str);

// string的编码方式为除utf8外的其它编码方式，可采用：
std::string wstring2string(const std::wstring& str, const std::string& locale);

std::wstring string2wstring(const std::string& str, const std::string& locale);

void hideCursorDisplay();

void gotoXY(int x, int y);

void setColor(unsigned short ForeColor = 15, unsigned short BackGroundColor = 0);


