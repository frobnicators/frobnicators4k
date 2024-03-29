#include "winclude.h"
#include "util.h"
#include "klister.h"
#include "shader.h"
#include "demo.h"
#include "music.h"

#if _DEBUG
#include <stdio.h>
#endif

#if TESTING
#include "test.h"
#endif

#define FRAME_RATE 60
#define MIN_DT (SAMPLE_RATE/FRAME_RATE)

static HDC		hDC; 
static HWND	hWnd;
DWORD width, height;
float dt, time;

static void CreateGLWindow() {
	DWORD dwStyle;

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

#if FULLSCREEN
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if(ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			dwStyle = WS_OVERLAPPEDWINDOW;
		} else {
			dwStyle = WS_POPUP;
		}
	}
	ShowCursor(FALSE);
#else
	dwStyle = WS_OVERLAPPEDWINDOW;
#endif

	hWnd = CreateWindow("static", NULL, WS_VISIBLE | dwStyle, 0, 0, width, height, NULL, NULL, NULL, NULL);
	hDC = GetDC(hWnd);
	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
	wglMakeCurrent(hDC, wglCreateContext(hDC));
}

static void run() {
	int i=0;
	unsigned long ldt;

#if defined(STATIC_WIDTH)
	width = STATIC_WIDTH;
	height = STATIC_HEIGHT;
#elif FULLSCREEN
	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);
#else
	width = 800;
	height = 600;
#endif

	CreateGLWindow();
	initKlister();
	init_shaders();
	init_gl();
	init_demo();
	init_music();

	do {
		update_time(&ldt);
		render_demo();
		SwapBuffers(hDC);
		if (ldt < MIN_DT){
			Sleep((MIN_DT - ldt) / SAMPLE_RATE);
		}
	} while (time < DEMO_LENGTH && !GetAsyncKeyState(VK_ESCAPE));

#if FULLSCREEN
	ChangeDisplaySettings(NULL, 0);
	ShowCursor(TRUE);
#endif

	ExitProcess(0);
}

#if !(defined(SUBSYSTEM_WINDOWS) || defined(SUBSYSTEM_CONSOLE) || defined(SUBSYSTEM_CRINKLER))
#error Build configuration must define either SUBSYSTEM_WINDOWS, SUBSYSTEM_CONSOLE or SUBSYSTEM_CRINKLER
#endif

#ifdef SUBSYSTEM_WINDOWS
int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow){
	run();
}
#endif

#ifdef SUBSYSTEM_CRINKLER
void __stdcall WinMainCRTStartup() {
	run();
}
#endif

#ifdef SUBSYSTEM_CONSOLE
int main() {
	run();
}
#endif
