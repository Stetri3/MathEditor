#pragma once

#define DEBUG

#ifdef DEBUG
constexpr bool IS_DEBUG = true;
#else
constexpr bool IS_DEBUG = false
#endif // DEBUG


#ifdef DEBUG
#include <iostream>
#endif // DEBUG

#ifdef DEBUG
#define DebugMessage(...) std::cout << __VA_ARGS__ << "\n";
#else
#define DebugMessage(...) do {} while (0)
#endif

constexpr bool _SAFE = false;