#ifndef SECPLATFORM_SRC_COMMON_ENCODING_CONVERTER_H_
#define SECPLATFORM_SRC_COMMON_ENCODING_CONVERTER_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

#ifdef tstring
#error "\"tstring\" Macro has been defined."
#else
#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif
#endif

namespace EncodingConverter
{
	static int AnsiStrToWideStr(std::string& strSrc, std::wstring& strDest)
	{
		int nLen = strSrc.length() + 1;
		int nRet = 0;

		nLen *=  sizeof(wchar_t);

		wchar_t* pszW = new wchar_t[nLen];
		memset(pszW, 0, nLen);

		nRet = MultiByteToWideChar(CP_ACP, 0, strSrc.c_str(), -1, pszW, nLen); 

		strDest = pszW;
		delete[] pszW;

		return nRet;
	};

	static int WideStrToAnsiStr(std::wstring& strSrc, std::string& strDest)
	{
		int nLen = strSrc.length() + 1;
		int nRet = 0;

		nLen *= sizeof(wchar_t);

		char* pszA = new char[nLen];
		memset(pszA, 0, nLen);


		nRet = WideCharToMultiByte(CP_ACP, 0, strSrc.c_str(), -1, pszA, nLen, NULL, NULL); 

		strDest = pszA;
		delete[] pszA;

		return nRet;
	};

	static int AnsiStrToTStr(std::string& strSrc, std::tstring& strDest)
	{
		int nRet = 0;

#ifdef _UNICODE
		nRet = AnsiStrToWideStr(strSrc, strDest);
#else
		strDest = strSrc;
		nRet = strDest.length();
#endif

		return nRet;
	};

	static int TStrToAnsiStr(std::tstring& strSrc, std::string& strDest)
	{
		int nRet = 0;

#ifdef _UNICODE
		nRet = WideStrToAnsiStr(strSrc, strDest);
#else
		strDest = strSrc;
		nRet = strDest.length();
#endif

		return nRet;
	};

	static int WideStrToTStr(std::wstring& strSrc, std::tstring& strDest)
	{
		int nRet = 0;

#ifdef _UNICODE
		strDest = strSrc;
		nRet = strDest.length();
#else
		nRet = WideStrToAnsiStr(strSrc, strDest);
#endif

		return nRet;
	};

	static int TStrToWideStr(std::tstring& strSrc, std::wstring& strDest)
	{
		int nRet = 0;

#ifdef _UNICODE
		strDest = strSrc;
		nRet = strDest.length();
#else
		nRet = AnsiStrToWideStr(strSrc, strDest);
#endif

		return nRet;
	};

	static std::string ToAnsiString(const wchar_t* lpStr)
	{
		std::wstring wide_string = lpStr;
		std::string ansi_string;

		WideStrToAnsiStr(wide_string, ansi_string);
		return ansi_string;
	};

	static std::string ToAnsiString(const char* lpStr)
	{
		return std::string(lpStr);
	};

	static std::wstring ToWideString(const wchar_t* lpStr)
	{
		return std::wstring(lpStr);
	};

	static std::wstring ToWideString(const char* lpStr)
	{
		std::string ansi_string = lpStr;
		std::wstring wide_string;

		AnsiStrToWideStr(ansi_string, wide_string);
		return wide_string;
	};

	static std::tstring ToTString(const char* lpStr)
	{
#ifdef _UNICODE
		return ToWideString(lpStr);
#else
		return ToAnsiString(lpStr);
#endif
	};

	static std::tstring ToTString(const wchar_t* lpStr)
	{
#ifdef _UNICODE
		return ToWideString(lpStr);
#else
		return ToAnsiString(lpStr);
#endif
	};

    static int WideStrToUtf8Str(std::wstring& strSrc, std::string& strDest)
    {
        int nRet = 0;
        int nLen = 0;

        nLen = WideCharToMultiByte(CP_UTF8, 0, strSrc.c_str(), -1, NULL, 0, NULL, NULL);

        char * lpUtf8Str = new char[nLen+1];
        memset(lpUtf8Str, 0, nLen);
        nRet = WideCharToMultiByte(CP_UTF8, 0, strSrc.c_str(), -1, lpUtf8Str, nLen, NULL, NULL);
        strDest = lpUtf8Str;
        delete[] lpUtf8Str;

        return nRet;
    };

    static int AnsiStrToUtf8Str(std::string& strSrc, std::string& strDest)
    {
        int nRet = 0;
		std::wstring wide_string;

        nRet = AnsiStrToWideStr(strSrc, wide_string);
        nRet = WideStrToUtf8Str(wide_string, strDest);

		return nRet;
    };

    static int Utf8StrToWideStr(const std::string& strSrc, std::wstring& strDest)
    {
        int nRet = 0;
        int nLen = 0;

        nLen = MultiByteToWideChar(CP_UTF8, 0, strSrc.c_str(), -1, NULL, 0);

        wchar_t* lpWideStr = new wchar_t[nLen];
        memset(lpWideStr, 0, nLen*sizeof(lpWideStr[0]));
        nRet = MultiByteToWideChar(CP_UTF8, 0, strSrc.c_str(), -1, lpWideStr, nLen);
        strDest = lpWideStr;
        delete[] lpWideStr;

        return nRet;
    };

    static int Utf8StrToAnsiStr(const std::string& strSrc, std::string& strDest)
    {
        int nRet = 0;
		std::wstring wide_string;

        nRet = Utf8StrToWideStr(strSrc, wide_string);
        nRet = WideStrToAnsiStr(wide_string, strDest);

		return nRet;
    };    

	static int Utf8StrToTStr(const std::string& strSrc, std::tstring& strDest)
	{
#ifdef UNICODE
		return Utf8StrToWideStr(strSrc, strDest);
#else
		return Utf8StrToAnsiStr(strSrc, strDest);
#endif
	};    

    static std::string ToUtf8String(const std::string& str)
    {
        std::string ansi_string = str;
		std::string utf8_string;

		AnsiStrToUtf8Str(ansi_string, utf8_string);
		return utf8_string;
    };

    static std::string ToUtf8String(const std::wstring& str)
    {
		std::wstring wide_string = str;
        std::string utf8_string;

        WideStrToUtf8Str(wide_string, utf8_string);
		return utf8_string;
    };
};

#endif // SECPLATFORM_SRC_COMMON_ENCODING_CONVERTER_H_
