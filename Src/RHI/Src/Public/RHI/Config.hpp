#pragma once

// Platform-specific macros for importing/exporting symbols
#if defined(R_WIN32)

	#ifdef RHI_EXPORT
	    // We are building the library, so export the symbols
	#define RHI_API __declspec(dllexport)
	#else
	    // We are using the library, so import the symbols
	#define RHI_API __declspec(dllimport)
	#endif

#else
#define RHI_API
#endif