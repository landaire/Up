//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef NOWIDE_CONFIG_H
#define NOWIDE_CONFIG_H

#if defined(__WIN32) || defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#	if defined(DLL_EXPORT)
#		if defined(NOWIDE_SOURCE)
#			define NOWIDE_API __declspec(dllexport)
#		else
#			define NOWIDE_API __declspec(dllimport)
#		endif
#	else
#		define NOWIDE_API
#	endif
#else // ELF BINARIES
#	if defined(NOWIDE_SOURCE) && defined(NOWIDE_VISIBILITY_SUPPORT)
#		define NOWIDE_API __attribute__ ((visibility("default")))
#	else
#		define NOWIDE_API
#	endif
#endif

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32)) && !defined(__CYGWIN__)
#define NOWIDE_WIN_NATIVE
#endif

#if defined(_MSC_VER)
#define NOWIDE_MSVC
#endif

#endif /// NOWIDE_CONFIG_H
