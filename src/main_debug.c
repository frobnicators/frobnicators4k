#include "winclude.h"
#include "util.h"
#include "klister.h"
#include "shader.h"
#include "demo.h"
#include "music.h"
#include "debug.h"

#if _DEBUG
#include <stdio.h>
#endif

#if TESTING
#include "test.h"
#endif

static HDC		hDC; 
static HGLRC	hRC; 
static HWND	hWnd;
static HINSTANCE hInstance;
DWORD width, height;
float time;

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void __declspec(noreturn) terminate() {

#if FULLSCREEN
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(TRUE);
#endif

	UnregisterClass("OpenGL", hInstance);

	ExitProcess(0);
}

static void CreateGLWindow() {
	GLuint PixelFormat;
	WNDCLASS wc;
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT WindowRect = {0, 0, width, height } ;

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

	hInstance = GetModuleHandle(NULL);
	memset(&wc, 0, sizeof(wc));
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "OpenGL";

	if(!RegisterClass(&wc)) {
		FROB_ERROR("Startup error", "Failed to register the window class.");
		terminate();
	}

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
#if _DEBUG
			FROB_PRINTF("Failed to activate fullscreen.\n");
#endif
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			dwStyle = WS_OVERLAPPEDWINDOW;
		} else {
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP;
			ShowCursor(FALSE);
		}
	}
#else
	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	dwStyle = WS_OVERLAPPEDWINDOW;
#endif

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);
	/* This hack be pretty ... */
	if ((
		hWnd=CreateWindowEx(  dwExStyle,
				"OpenGL",
				"",
				WS_CLIPSIBLINGS |
				WS_CLIPCHILDREN |
				dwStyle,
				0, 0,
				WindowRect.right - WindowRect.left,
				WindowRect.bottom - WindowRect.top,
				NULL,
				NULL,
				hInstance,
				NULL)
				) == NULL) {
		FROB_PRINTF("Failed to create window: %d\n", GetLastError());
		terminate();
	}

	
	if (!(hDC=GetDC(hWnd))) {
		FROB_PRINTF("Can't Create A GL Device Context: %d", GetLastError());
		terminate(); 
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd))) {
		FROB_PRINTF("Can't Find A Suitable PixelFormat: %d", GetLastError());
		terminate();
	}

	if(!SetPixelFormat(hDC, PixelFormat, &pfd)) {
		FROB_PRINTF("Can't set the pixel format: %d", GetLastError());
		terminate();
	}

	if(!(hRC = wglCreateContext(hDC))) {
		FROB_PRINTF("Can't create a opengl context: %d", GetLastError());
		terminate();
	}

	if(!wglMakeCurrent(hDC, hRC)) {
		FROB_PRINTF("Failed to make context current: %d", GetLastError());
		terminate();
	}

	ShowWindow(hWnd,SW_SHOW);
	//SetForegroundWindow(hWnd);
	//SetFocus(hWnd);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
	case WM_SYSCOMMAND:
		switch(wParam) {
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0; /* Prevent screensavers and powersave mode */
		}
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		terminate();
		break;
	case WM_KEYDOWN:
		if(wParam == VK_ESCAPE) terminate();
		break;
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

static void initGL() {
	initKlister();
	init_shaders();
#if INIT_GL_NEEDED
	init_gl();
#endif
}

static void do_the_magic() {
	MSG msg;
	while(1) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			update_time();
			if(time >= DEMO_LENGTH) terminate();
#if _DEBUG
			FROB_PRINTF("Time: %f\n", time);
#endif
			render_demo();
			SwapBuffers(hDC);
		}
	}
}

static void run() {
	int i=0;
#if defined(STATIC_WIDTH)
	width = STATIC_WIDTH;
	height = STATIC_HEIGHT;
#else
#if FULLSCREEN
	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);
#else
	width = 800;
	height = 600;
#endif
#endif
	FROB_PRINTF("Starting frob4k\n");
	CreateGLWindow();
	initGL();
	init_demo();

	init_music();

	do_the_magic();
}

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow){
#if TESTING
	run_tests();
#else
	run();
#endif
}
