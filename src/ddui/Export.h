#pragma once

#ifdef DDUI_EXPORT
#define DDUI_API __declspec(dllexport)
#else
#define DDUI_API __declspec(dllimport)
#endif