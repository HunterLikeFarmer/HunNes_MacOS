#pragma once
// This file is for SDL dependencies
#if defined(__has_include)
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#elif __has_include(<SDL.h>)
#include <SDL.h>
#else
#error "SDL2 headers not found. Install SDL2 development package."
#endif
#else
#include <SDL2/SDL.h>
#endif
