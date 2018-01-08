#pragma once

/**
 * @file Debugging.hpp
 * @brief Debugging header file.
 * @author Connor J. Bramham (ReeCocho)
 */

/** Includes. */
#include <iostream>

#define gLog(MSG) std::clog << MSG

// Debugging
#ifdef NDEBUG
	#define gErr(MSG) ((void)0)
	#define gOut(MSG) ((void)0)
	#define gAssert(COND) ((void)0)
#else
	#define gErr(MSG) { std::cerr << MSG; std::terminate(); }
	#define gOut(MSG) std::cout << MSG
	#define gAssert(COND) {if(!(COND)) gErr("Assertation failed: " << #COND << '\n'); }
#endif