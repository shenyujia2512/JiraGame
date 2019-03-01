#include "Utilitys.h"

std::string GenrateRandomID()
{
	//随机生成一个session_id
	string CServerRandomID = "";
	char randNum[6][4];

	for (int i = 0; i < 6; i++)
	{
		sprintf_s(randNum[i], "%d", rand() % 10 + 1);
		CServerRandomID += randNum[i];
	}

	return CServerRandomID;
}

void hideCursorDisplay()
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cci;
	GetConsoleCursorInfo(hOut, &cci);
	cci.bVisible = false;
	SetConsoleCursorInfo(hOut, &cci);
}

void gotoXY(int x, int y)
{
	COORD Position = { y, x };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Position);
}

void setColor(unsigned short ForeColor, unsigned short BackGroundColor)
{
	/*
	0 - BLACK
	1 - BLUE          2 - GREEN
	3 - CYAN          4 - RED
	5 - MAGENTA       6 - BROWN
	7 - LIGHTGRAY     8 - DARKGRAY
	9 - LIGHTBLUE     10 - LIGHTGREEN
	11 - LIGHTCYAN    12 - LIGHTRED
	13 - LIGHTMAGENTA 14 - YELLOW
	15 - WHITE
	*/
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, ForeColor | BackGroundColor);
}


std::string wstring2utf8string(const std::wstring& str)
{
	static std::wstring_convert<std::codecvt_utf8<wchar_t> > strCnv;
	return strCnv.to_bytes(str);
}

std::wstring utf8string2wstring(const std::string& str)
{
	static std::wstring_convert< std::codecvt_utf8<wchar_t> > strCnv;
	return strCnv.from_bytes(str);
}

std::string wstring2string(const std::wstring& str, const std::string& locale)
{
	typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> F;
	static std::wstring_convert<F> strCnv(new F(locale));

	return strCnv.to_bytes(str);
}

std::wstring string2wstring(const std::string& str, const std::string& locale)
{
	typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> F;
	static std::wstring_convert<F> strCnv(new F(locale));

	return strCnv.from_bytes(str);
}

std::string int2String(int iValue)
{
	char _chr[32];
	sprintf_s(_chr, "%d", iValue);

	return _chr;
}

int string2Int(string str)
{
	return atoi(str.c_str());
}

