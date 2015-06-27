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
static HGLRC	hRC; 
static HWND	hWnd;
static HINSTANCE hInstance;
DWORD width, height;
float dt, time;

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

#if _DEBUG
	if(!RegisterClass(&wc)) {
		MessageBox(NULL, "Failed to register the window class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		terminate();
	}
#else
	RegisterClass(&wc);
#endif

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
			printf("Failed to activate fullscreen.\n");
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
#if _DEBUG
	if ((
#endif
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
#if _DEBUG 
				) == NULL) {
		printf("Failed to create window: %d\n", GetLastError());
		terminate();
	}
#else
	;
#endif

	
#if _DEBUG
	if (!(hDC=GetDC(hWnd))) {
		printf("Can't Create A GL Device Context: %d", GetLastError());
		terminate(); 
	}
#else
	hDC = GetDC(hWnd);
#endif

#if _DEBUG
	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd))) {
		printf("Can't Find A Suitable PixelFormat: %d", GetLastError());
		terminate();
	}
#else
	PixelFormat=ChoosePixelFormat(hDC,&pfd);
#endif

#if _DEBUG
	if(!SetPixelFormat(hDC, PixelFormat, &pfd)) {
		printf("Can't set the pixel format: %d", GetLastError());
		terminate();
	}
#else
	SetPixelFormat(hDC, PixelFormat, &pfd);
#endif

#if _DEBUG
	if(!(hRC = wglCreateContext(hDC))) {
		printf("Can't create a opengl context: %d", GetLastError());
		terminate();
	}
#else
	hRC = wglCreateContext(hDC);
#endif

#if _DEBUG
	if(!wglMakeCurrent(hDC, hRC)) {
		printf("Failed to make context current: %d", GetLastError());
		terminate();
	}
#else
	wglMakeCurrent(hDC, hRC);
#endif

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

	if (wParam == VK_ESCAPE){
	case WM_KEYDOWN:
		; /* fall through */
	}
	case WM_DESTROY:
		terminate();
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
	unsigned long ldt;

	while(1) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			update_time(&ldt);
			if(time >= DEMO_LENGTH) terminate();
#if _DEBUG
			printf("Time: %f, %ld\n", time, ldt);
#endif
			render_demo();
			SwapBuffers(hDC);
			if(ldt < MIN_DT) Sleep( ( MIN_DT - ldt) / SAMPLE_RATE);
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
	CreateGLWindow();
	initGL();
	init_demo();

	init_music();

	do_the_magic();
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
#if TESTING
	run_tests();
#else
	run();
#endif
}
#endif
