#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <GL/glew.h>
#include "util.h"

HDC		hDC = NULL; 
HGLRC	hRC = NULL; 
HWND	hWnd = NULL;
HINSTANCE hInstance = NULL;

BOOL    keys[256];
BOOL    active=TRUE;
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

	exit(0);
}

void CreateGLWindow(const char * title, int width, int height) {
	GLuint PixelFormat;
	WNDCLASS wc = { 0 };
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
		//memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if(ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
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
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void run() {

}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow) {
	CreateGLWindow("Frobnicators 4k", 800, 600);
	 
	Sleep(2000);

	terminate();
}

int main() {
	CreateGLWindow("Frobnicators 4k", 800, 600);
	 
	Sleep(2000);

	terminate();
}