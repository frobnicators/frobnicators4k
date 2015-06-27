#ifndef MAIN_H
#define MAIN_H
#include "winclude.h"
#if _DEBUG || SOME_DEBUG
	void __declspec(noreturn) terminate();
#endif
	extern float dt, time;
	extern DWORD width, height;
#endif