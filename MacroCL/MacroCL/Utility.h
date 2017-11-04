#pragma once
#ifndef _UTILITY_H_
#define _UTILITY_H

#include "stdafx.h"

#define SafeDelete(x) {if((x) != nullptr) { delete (x); } x = nullptr; }
#define SafeDeleteArray(x) {if((x) != nullptr) { delete[] (x); } x = nullptr; }

typedef sf::Image			SFImage;
typedef sf::Texture		SFTexture;

namespace Util
{
	inline void PrintLog(const std::wstring msg) { std::wcout << msg; }
	inline void PrintLog(const std::string msg) { std::cout << msg; }
	inline void PrintLogLine(const std::wstring msg) { std::wcout << msg << std::endl; }
	inline void PrintLogLine(const std::string msg) { std::cout << msg << std::endl; }

	class NonCopyable
	{
	public:
		NonCopyable() {};
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator=(const NonCopyable&) = delete;
	};

	static std::wstring StrToWStr(const std::string& str)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.from_bytes(str);
	}

	static std::string WStrToStr(const std::wstring& wstr)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.to_bytes(wstr);
	}

	static std::wstring ReadWholeFile(const std::wstring& path)
	{
		std::wifstream in(WStrToStr(path));
		return std::wstring(std::istreambuf_iterator<wchar_t>(in), std::istreambuf_iterator<wchar_t>());
	}
}

#endif