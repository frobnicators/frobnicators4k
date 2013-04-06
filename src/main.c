#include "winclude.h"
#include <GL/GL.h>
#include "util.h"
#include "klister.h"

HDC		hDC = NULL; 
HGLRC	hRC = NULL; 
HWND	hWnd = NULL;
HINSTANCE hInstance = NULL;

BOOL    fullscreen = FULLSCREEN;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void terminate() {

	if(fullscreen) {
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(TRUE);
	}

	if(hRC) {
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hRC);
		hRC = NULL;
	}

	if(hDC) ReleaseDC(hWnd, hDC);
	hDC = NULL;

	if(hWnd) DestroyWindow(hWnd);
	hWnd = NULL;

	UnregisterClass("OpenGL", hInstance);

	ExitProcess(0);
}

void CreateGLWindow(const char * title, int width, int height) {
	GLuint PixelFormat;
	WNDCLASS wc;
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT WindowRect;

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	WindowRect.left = (long) 0;
	WindowRect.right = (long) width;
	WindowRect.top = (long) 0;
	WindowRect.bottom = (long) height;

	hInstance = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "OpenGL";

	if(!RegisterClass(&wc)) {
		MessageBox(NULL, "Failed to register the window class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		terminate();
	}

	if(fullscreen) {
		DEVMODE dmScreenSettings = { 0 };
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if(ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
#if _DEBUG
			debug("Failed to activate fullscreen.\n");
#endif
			fullscreen = FALSE;
		} else {
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP;
			ShowCursor(FALSE);
		}
	}

	if(!fullscreen) {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	if ((hWnd=CreateWindowEx(  dwExStyle,
				"OpenGL",
				title,
				WS_CLIPSIBLINGS |
				WS_CLIPCHILDREN |
				dwStyle,
				0, 0,
				WindowRect.right - WindowRect.left,
				WindowRect.bottom - WindowRect.top,
				NULL,
				NULL,
				hInstance,
				NULL)) == NULL)
	{
#if _DEBUG
		debug("Failed to create window: %d\n", GetLastError());
#endif
		terminate();
	}

	
	if (!(hDC=GetDC(hWnd))) {
		#if _DEBUG
		debug("Can't Create A GL Device Context: %d", GetLastError());
		#endif
		terminate(); 
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd))) {
		#if _DEBUG
		debug("Can't Find A Suitable PixelFormat: %d", GetLastError());
		#endif
		terminate();
	}
	
	if(!SetPixelFormat(hDC, PixelFormat, &pfd)) {
		#if _DEBUG
		debug("Can't set the pixel format: %d", GetLastError());
		#endif
		terminate();
	}

	if(!(hRC = wglCreateContext(hDC))) {
		#if _DEBUG
		debug("Can't create a opengl context: %d", GetLastError());
		#endif
		terminate();
	}

	if(!wglMakeCurrent(hDC, hRC)) {
		#if _DEBUG
		debug("Failed to make context current: %d", GetLastError());
		#endif
		terminate();
	}

	ShowWindow(hWnd,SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if(wParam == VK_ESCAPE) PostQuitMessage(0);
		break;
	/* case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam)); //width, height
		break; */
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void initGL() {
	if(!initKlister()) {
#if _DEBUG
		debug("Failed to init opengl glue\n");
		terminate();
#endif
	}
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

void do_the_magic() {
	float dt, t;
	MSG msg;
	BOOL done = FALSE;

	while(!done) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if(msg.message == WM_QUIT) {
				done = TRUE;
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else {
			t = get_time(&dt);
			glClearColor(1.f, 0.f, 1.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			SwapBuffers(hDC);
			Sleep(100);
		}
	}
}

void run() {
	DWORD dwWidth = 800;
	DWORD dwHeight = 600;
	int i=0;

	if(fullscreen) {
		dwWidth = GetSystemMetrics(SM_CXSCREEN);
		dwHeight = GetSystemMetrics(SM_CYSCREEN);
	}
	CreateGLWindow("Frobnicators 4k", dwWidth, dwHeight);
	initGL();
	start_time();

	do_the_magic();
	 
	terminate();
}

/* For release */
void __stdcall WinMainCRTStartup() {
	run();		
}

/* For debug */
int main() { run(); }
