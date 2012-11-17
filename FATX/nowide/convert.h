#ifndef NOWIDE_NOWIDE_CONVERT_H
#define NOWIDE_NOWIDE_CONVERT_H

#include "config.h"
#include <stdexcept>

#ifdef NOWIDE_WIN_NATIVE
namespace nowide {

	class NOWIDE_API bad_utf : public std::runtime_error {
	public:
		bad_utf();
	};

	NOWIDE_API std::string convert(wchar_t const *s);
	NOWIDE_API std::wstring convert(char const *s);
	inline std::string convert(std::wstring const &s) 
	{
		return convert(s.c_str());
	}
	inline std::wstring convert(std::string const &s) 
	{
		return convert(s.c_str());
	}

} // nowide
#endif

#endif
