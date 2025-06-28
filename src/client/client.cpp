#include "innerstat/client/main_frame.h"
#include "innerstat/client/canvas.h"
#include <tchar.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_TREEVIEW_CLASSES };
    InitCommonControlsEx(&icex);

    // 캔버스 윈도우 클래스 등록 (더블클릭 포함)
    WNDCLASS wccanvas = {};
    wccanvas.style         = CS_DBLCLKS;
    wccanvas.lpfnWndProc   = CanvasProc;
    wccanvas.hInstance     = hInstance;
    wccanvas.lpszClassName = _T("CanvasWindow");
    wccanvas.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wccanvas);

    // 메인 윈도우 클래스 등록
    const TCHAR CLASS_NAME[] = _T("MyWin32Window");
    WNDCLASS wc = {};
    wc.lpfnWndProc   = MainWndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, _T("InnerStat"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 600,
        nullptr, nullptr, hInstance, nullptr
    );

    if (hwnd == nullptr)
        return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
