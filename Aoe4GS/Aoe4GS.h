#pragma once
#include "pch.h"

#ifdef AOE4GS_EXPORTS
#define AOE4_GS_API __declspec(dllexport)
#else
#define AOE4_GS_API __declspec(dllimport)
#endif

extern "C" {

	int AOE4_GS_API AGS_Init(HWND hwnd);
	unsigned int AOE4_GS_API AGS_GetColor(int x,int y);
	void AOE4_GS_API AGS_Stop();
}