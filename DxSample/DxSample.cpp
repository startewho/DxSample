// DxSample.cpp : Defines the entry point for the application.
//
#include "Common.h"
#include "DxSample.h"
#include "VAReaderWriterTranscoder.h"
#include "Player.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]; 
// the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);



//自定义字段
CPlayer     *g_pPlayer = NULL;
VACReaderWriterTranscoder* pTranscoder;
CComPtr<IMFMediaSource> mixSource;
HWND mHWND;
HANDLE hThread;



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DXSAMPLE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DXSAMPLE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

DWORD WINAPI ClientThread(LPVOID lpParam)
{
	VACReaderWriterTranscoder* _pCoder = (VACReaderWriterTranscoder*)lpParam;

	_pCoder->StatrtCapture(L".\\out.mp4", mHWND);

	return 0;
}

HRESULT StartCap(HWND hwnd)
{
	
	HRESULT hr = S_OK;
	
	DWORD dwThread;

	

	pTranscoder = new VACReaderWriterTranscoder();
	

	pTranscoder->Capting = true;

	hThread=CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            ClientThread,       // thread function name
            (LPVOID)pTranscoder,          // argument to thread function 
            0,                      // use default creation flags 
            &dwThread);   // returns the thread identifier 
	
	pTranscoder->GetMixSource(&mixSource);

	g_pPlayer = new (std::nothrow) CPlayer(hwnd, &hr);

	g_pPlayer->OpenSource(mixSource);


	g_pPlayer->Play();

	return hr;

}


HRESULT StopCap(HWND hwnd)
{
	HRESULT hr = S_OK;

	

 
	pTranscoder->Capting = false;

	if (g_pPlayer)
	{

		
		g_pPlayer->Stop();
	

	}

	WaitForSingleObject(hThread, 1000);

	pTranscoder->StopCature();
	
	CloseHandle(hThread);

	

	delete g_pPlayer;

	delete pTranscoder;

	return hr;
}



HRESULT StartPrtScn(ImageType imageType)
{
	HRESULT hr = S_OK;

	pTranscoder->_imageType = imageType;
	pTranscoder->PrtScn = true;


	return hr;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DXSAMPLE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DXSAMPLE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);


   mHWND = hWnd;
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);



   return TRUE;
}

void OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	if (g_pPlayer&&g_pPlayer->HasVideo())
	{
		// We have a player with an active topology and a video renderer that can paint the
		// window surface - ask the videor renderer (through the player) to redraw the surface.
		g_pPlayer->Repaint();
	}
	else
	{
		// The player topology hasn't been activated, which means there is no video renderer that 
		// repaint the surface.  This means we must do it ourselves.
		RECT rc;
		GetClientRect(hwnd, &rc);
		FillRect(hdc, &rc, (HBRUSH)COLOR_WINDOW);
	}
	EndPaint(hwnd, &ps);
}
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
				break;
			case IDM_START:
				StartCap(hWnd);
				break;
			case IDM_STOP:
				StopCap(hWnd);
				break;
			case IDM_PRTSCN_BMP:
				StartPrtScn(BMP);
				break;
			case IDM_PRTSCN_JPG:
				StartPrtScn(JPG);
				break;
			case IDM_PRTSCN_PNG:
				StartPrtScn(PNG);
				break;
		
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
		
			OnPaint(hWnd);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	
	case WM_SIZE:
		if (g_pPlayer != NULL)
		{
			LONG wdith = LOWORD(lParam); // resizing flag
			LONG heigth = HIWORD(lParam);
			RECT rect;
			rect.top = 0.0;
			rect.left = 0.0;
			rect.right = wdith;
			rect.bottom = heigth;
			g_pPlayer->Resize(&rect);
		}
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
