// cr3win.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "cr3win.h"

#include <lvstring.h>

#include <GL/glew.h>
#include <GL/wglew.h>
#include <string>

#define MAX_LOADSTRING 100

#define WINDOW_VSYNC        1

static HWND __hwnd = 0;
static HINSTANCE __hinstance = 0;
static HWND __attachToWindow = 0;
static HDC __hdc = 0;
static bool __vsync = WINDOW_VSYNC;
static HGLRC __hrc = 0;
static bool __multiSampling = false;

// Window defaults
#define DEFAULT_RESOLUTION_X 1024
#define DEFAULT_RESOLUTION_Y 768
#define DEFAULT_COLOR_BUFFER_SIZE 32
#define DEFAULT_DEPTH_BUFFER_SIZE 24
#define DEFAULT_STENCIL_BUFFER_SIZE 8

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

struct WindowCreationParams
{
    RECT rect;
    std::wstring windowName;
    bool fullscreen;
    bool resizable;
    int samples;
};

bool initializeGL(WindowCreationParams* params);

/**
 * Gets the width and height of the screen in pixels.
 */
static void getDesktopResolution(int& width, int& height)
{
   RECT desktop;
   const HWND hDesktop = GetDesktopWindow();
   // Get the size of screen to the variable desktop
   GetWindowRect(hDesktop, &desktop);
   width = desktop.right;
   height = desktop.bottom;
}



int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	CRLog::setFileLogger("cr3.log", true);
	CRLog::setLogLevel(CRLog::LL_TRACE);
	CRLog::info("WinMain");

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	__hinstance = hInstance;

    WindowCreationParams params;
    params.fullscreen = false;
    params.resizable = true;
    params.rect.left = 0;
    params.rect.top = 0;
    params.rect.right = 500;
    params.rect.bottom = 400;
    params.samples = 0;

    if (params.rect.right == 0)
        params.rect.right = params.rect.left + DEFAULT_RESOLUTION_X;
    if (params.rect.bottom == 0)
        params.rect.bottom = params.rect.top + DEFAULT_RESOLUTION_Y;
    int width = params.rect.right - params.rect.left;
    int height = params.rect.bottom - params.rect.top;

    if (params.fullscreen)
    {
        // Enumerate all supposed display settings
        bool modeSupported = false;
        DWORD modeNum = 0;
        DEVMODE devMode;
        memset(&devMode, 0, sizeof(DEVMODE));
        devMode.dmSize = sizeof(DEVMODE);
        devMode.dmDriverExtra = 0;
        while (EnumDisplaySettings(NULL, modeNum++, &devMode) != 0)
        {
            // Is mode supported?
            if (devMode.dmPelsWidth == width &&
                devMode.dmPelsHeight == height &&
                devMode.dmBitsPerPel == DEFAULT_COLOR_BUFFER_SIZE)
            {
                modeSupported = true;
                break;
            }
        }

        // If the requested mode is not supported, fall back to a safe default
        if (!modeSupported)
        {
            width = DEFAULT_RESOLUTION_X;
            height = DEFAULT_RESOLUTION_Y;
            params.rect.right = params.rect.left + width;
            params.rect.bottom = params.rect.top + height;
        }
    }


 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_CR3WIN, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!initializeGL(&params))
        exit(1);

    ShowWindow(__hwnd, SW_SHOW);

	//// Perform application initialization:
	//if (!InitInstance (hInstance, nCmdShow))
	//{
	//	return FALSE;
	//}


	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CR3WIN));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

bool createWindow(WindowCreationParams* params, HWND* hwnd, HDC* hdc)
{
    bool fullscreen = false;
    bool resizable = false;
    RECT rect = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT };
    std::wstring windowName;
    if (params)
    {
        windowName = params->windowName;
        memcpy(&rect, &params->rect, sizeof(RECT));
        fullscreen = params->fullscreen;
        resizable = params->resizable;
    }

    // Set the window style.
    DWORD style, styleEx;
    if (fullscreen)
    {
        style = WS_POPUP;
        styleEx = WS_EX_APPWINDOW;
    }
    else
    {
        if (resizable)
            style = WS_OVERLAPPEDWINDOW;
        else
            style = WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU;
        styleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    }
    style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    // Adjust the window rectangle so the client size is the requested size.
    AdjustWindowRectEx(&rect, style, FALSE, styleEx);

    // Create the native Windows window.
    *hwnd = CreateWindowEx(styleEx, szWindowClass, windowName.c_str(), style, 0, 0, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, __hinstance, NULL);
    if (*hwnd == NULL)
    {
        CRLog::error("Failed to create window.");
        return false;
    }

    // Get the drawing context.
    *hdc = GetDC(*hwnd);
    if (*hdc == NULL)
    {
        CRLog::error("Failed to get device context.");
        return false;
    }

    // Center the window
    GetWindowRect(*hwnd, &rect);
    const int screenX = (GetSystemMetrics(SM_CXSCREEN) - rect.right) / 2;
    const int screenY = (GetSystemMetrics(SM_CYSCREEN) - rect.bottom) / 2;
	CRLog::info("screen size %d x %d", screenX, screenY);
    SetWindowPos(*hwnd, *hwnd, screenX, screenY, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    return true;
}

bool initializeGL(WindowCreationParams* params)
{
    // Create a temporary window and context to we can initialize GLEW and get access
    // to additional OpenGL extension functions. This is a neccessary evil since the
    // function for querying GL extensions is a GL extension itself.
    HWND hwnd = NULL;
    HDC hdc = NULL;

    if (params)
    {
        if (!createWindow(NULL, &hwnd, &hdc))
            return false;
    }
    else
    {
        hwnd = __hwnd;
        hdc = __hdc;
    }

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize  = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = DEFAULT_COLOR_BUFFER_SIZE;
    pfd.cDepthBits = DEFAULT_DEPTH_BUFFER_SIZE;
    pfd.cStencilBits = DEFAULT_STENCIL_BUFFER_SIZE;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (pixelFormat == 0)
    {
        DestroyWindow(hwnd);
        CRLog::error("Failed to choose a pixel format.");
        return false;
    }

    if (!SetPixelFormat(hdc, pixelFormat, &pfd))
    {
        DestroyWindow(hwnd);
        CRLog::error("Failed to set the pixel format.");
        return false;
    }

    HGLRC tempContext = wglCreateContext(hdc);
    if (!tempContext)
    {
        DestroyWindow(hwnd);
        CRLog::error("Failed to create temporary context for initialization.");
        return false;
    }
    wglMakeCurrent(hdc, tempContext);

    // Initialize GLEW
    if (GLEW_OK != glewInit())
    {
        wglDeleteContext(tempContext);
        DestroyWindow(hwnd);
        CRLog::error("Failed to initialize GLEW.");
        return false;
    }

    // Choose pixel format using wglChoosePixelFormatARB, which allows us to specify
    // additional attributes such as multisampling.
    //
    // Note: Keep multisampling attributes at the start of the attribute lists since code below
    // assumes they are array elements 0 through 3.
    int attribList[] = {
        WGL_SAMPLES_ARB, params ? params->samples : 0,
        WGL_SAMPLE_BUFFERS_ARB, params ? (params->samples > 0 ? 1 : 0) : 0,
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, DEFAULT_COLOR_BUFFER_SIZE,
        WGL_DEPTH_BITS_ARB, DEFAULT_DEPTH_BUFFER_SIZE,
        WGL_STENCIL_BITS_ARB, DEFAULT_STENCIL_BUFFER_SIZE,
        0
    };
    __multiSampling = params && params->samples > 0;

    UINT numFormats;
    if ( !wglChoosePixelFormatARB(hdc, attribList, NULL, 1, &pixelFormat, &numFormats) || numFormats == 0)
    {
        bool valid = false;
        if (params && params->samples > 0)
        {
            CRLog::warn("Failed to choose pixel format with WGL_SAMPLES_ARB == %d. Attempting to fallback to lower samples setting.", params->samples);
            while (params->samples > 0)
            {
                params->samples /= 2;
                attribList[1] = params->samples;
                attribList[3] = params->samples > 0 ? 1 : 0;
                if (wglChoosePixelFormatARB(hdc, attribList, NULL, 1, &pixelFormat, &numFormats) && numFormats > 0)
                {
                    valid = true;
                    CRLog::error("Found pixel format with WGL_SAMPLES_ARB == %d.", params->samples);
                    break;
                }
            }

            __multiSampling = params->samples > 0;
        }

        if (!valid)
        {
            wglDeleteContext(tempContext);
            DestroyWindow(hwnd);
            CRLog::error("Failed to choose a pixel format.");
            return false;
        }
    }

    // Create new/final window if needed
    if (params)
    {
        DestroyWindow(hwnd);
        hwnd = NULL;
        hdc = NULL;

        if (!createWindow(params, &__hwnd, &__hdc))
        {
            wglDeleteContext(tempContext);
            return false;
        }
    }

    // Set final pixel format for window
    if (!SetPixelFormat(__hdc, pixelFormat, &pfd))
    {
        CRLog::error("Failed to set the pixel format: %d.", (int)GetLastError());
        return false;
    }

    // Create our new GL context
    int attribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        0
    };

    if (!(__hrc = wglCreateContextAttribsARB(__hdc, 0, attribs) ) )
    {
        wglDeleteContext(tempContext);
        CRLog::error("Failed to create OpenGL context.");
        return false;
    }

    // Delete the old/temporary context and window
    wglDeleteContext(tempContext);

    // Make the new context current
    if (!wglMakeCurrent(__hdc, __hrc) || !__hrc)
    {
        CRLog::error("Failed to make the window current.");
        return false;
    }

    // Vertical sync.
    wglSwapIntervalEXT(__vsync ? 1 : 0);

    return true;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CR3WIN));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; //(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; //MAKEINTRESOURCE(IDC_CR3WIN);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL; //LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   __hinstance = hInst;
   __hwnd = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //if (!game->isInitialized() || hwnd != __hwnd)
    //{
    //    // Ignore messages that are not for our game window.
    //    return DefWindowProc(hwnd, msg, wParam, lParam);
    //}

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	CRLog::trace("message %04x", message);
	switch (message)
	{
    case WM_CLOSE:
        DestroyWindow(__hwnd);
        return 0;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	//case WM_PAINT:
	//	hdc = BeginPaint(hWnd, &ps);
	//	// TODO: Add any drawing code here...
	//	EndPaint(hWnd, &ps);
	//	break;
	case WM_DESTROY:
		CRLog::info("WM_DESTROY");
        //gameplay::Platform::shutdownInternal();
		PostQuitMessage(0);
		break;
    case WM_SETFOCUS:
        break;

    case WM_KILLFOCUS:
        break;

    case WM_SIZE:
        // Window was resized.
        //gameplay::Platform::resizeEventInternal((unsigned int)(short)LOWORD(lParam), (unsigned int)(short)HIWORD(lParam));
        break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
