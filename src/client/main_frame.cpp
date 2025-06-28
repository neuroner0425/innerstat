#include "innerstat/client/main_frame.h"
#include "innerstat/client/canvas.h"
#include <windowsx.h>
#include <tchar.h>

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hTree = nullptr, hBtn = nullptr, hCanvas = nullptr;
    static int areaCount = 1;

    switch (msg) {
    case WM_CREATE:
        hTree = CreateWindowEx(0, WC_TREEVIEW, nullptr,
            WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
            0, 40, 200, 560, hwnd, (HMENU)103, nullptr, nullptr);

        hBtn = CreateWindowEx(0, _T("BUTTON"), _T("Add Area"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 200, 40, hwnd, (HMENU)104, nullptr, nullptr);

        hCanvas = CreateWindowEx(0, _T("CanvasWindow"), nullptr,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            200, 0, 800, 600, hwnd, (HMENU)105, nullptr, nullptr);
        break;

    case WM_SIZE: {
        int w = LOWORD(lParam), h = HIWORD(lParam);
        if (hTree && hBtn && hCanvas) {
            MoveWindow(hBtn, 0, 0, 200, 40, TRUE);
            MoveWindow(hTree, 0, 40, 200, h - 40, TRUE);
            MoveWindow(hCanvas, 200, 0, w - 200, h, TRUE);
        }
        break;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == 104) {
            // 트리뷰 추가 + 박스 동시 생성
            HTREEITEM item = nullptr;
            if (hTree) {
                TVINSERTSTRUCT tvis = {};
                tvis.hParent = TVI_ROOT;
                tvis.hInsertAfter = TVI_LAST;
                TCHAR buf[32];
                wsprintf(buf, _T("Area %d"), areaCount++);
                tvis.item.mask = TVIF_TEXT;
                tvis.item.pszText = buf;
                item = TreeView_InsertItem(hTree, &tvis);
            }
            // 캔버스에도 같은 treeItem 핸들로 박스 추가
            Canvas_AddBox(250, 50 + 80 * (int)g_boxes.size(), item);
            if (hCanvas) InvalidateRect(hCanvas, NULL, TRUE);
            break;
        }
        break;
    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->idFrom == 103 && ((LPNMHDR)lParam)->code == TVN_SELCHANGED) {
            LPNMTREEVIEW ptv = (LPNMTREEVIEW)lParam;
            HTREEITEM selectedItem = ptv->itemNew.hItem;
            Canvas_SelectBoxByTreeItem(selectedItem);
            if (hCanvas) InvalidateRect(hCanvas, NULL, TRUE);
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
