#include "winclude.h"
#include "util.h"
#include "klister.h"
#include "shader.h"
#include "demo.h"

static HDC		hDC; 
static HGLRC	hRC; 
static HWND	hWnd;
static HINSTANCE hInstance;
DWORD width, height;

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void __declspec(noreturn) terminate() {

#if FULLSCREEN
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(TRUE);
#endif

	UnregisterClass("OpenGL", hInstance);

	ExitProcess(0);
}

static void CreateGLWindow(const char * title) {
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
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
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
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if(wParam == VK_ESCAPE) PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

static void initGL() {
	initKlister();
	/*glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);*/

	init_shaders();
	init_demo();
}

static void do_the_magic() {
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
#if _DEBUG
			printf("Time: %f\n", t);
#endif
			render_demo(dt, t);
			SwapBuffers(hDC);
			Sleep(100);
		}
	}
}

static void run() {
	int i=0;
#if FULLSCREEN
	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);
#else
	width = 800;
	height = 600;
#endif
	CreateGLWindow(DEMO_NAME);
	initGL();
	start_time();

	do_the_magic();
	 
	terminate();
}

/* For release */
void __stdcall WinMainCRTStartup() {
	run();		
}

#if _DEBUG
/* For debug */
int main() { run(); }
#endif