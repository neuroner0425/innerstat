#include "innerstat/client/canvas.h"
#include <windowsx.h>

std::vector<Box> g_boxes;
double g_zoom = 1.0;
int g_offsetX = 0, g_offsetY = 0;

static bool g_panning = false;
static int g_panStartX, g_panStartY, g_viewStartX, g_viewStartY;
static int g_dragging = -1, g_dragOffsetX, g_dragOffsetY;

// 좌표 변환 함수
static int screenToWorldX(int x) { return int((x - g_offsetX) / g_zoom); }
static int screenToWorldY(int y) { return int((y - g_offsetY) / g_zoom); }
static int worldToScreenX(int x) { return int(x * g_zoom + g_offsetX); }
static int worldToScreenY(int y) { return int(y * g_zoom + g_offsetY); }

void Canvas_AddBox(int x, int y, HTREEITEM treeItem) {
    g_boxes.push_back({ x-50, y-30, 100, 60, false, treeItem });
}

void Canvas_ZoomTo(int mx, int my, double newZoom) {
    double wx = (mx - g_offsetX) / g_zoom;
    double wy = (my - g_offsetY) / g_zoom;
    g_zoom = newZoom;
    if (g_zoom < 0.2) g_zoom = 0.2;
    if (g_zoom > 5.0) g_zoom = 5.0;
    g_offsetX = mx - int(wx * g_zoom);
    g_offsetY = my - int(wy * g_zoom);
}

void Canvas_SelectBoxByTreeItem(HTREEITEM treeItem) {
    for (auto& box : g_boxes)
        box.selected = (box.treeItem == treeItem);
}

int Canvas_GetBoxIndexAt(int wx, int wy) {
    for (int i = (int)g_boxes.size() - 1; i >= 0; --i) {
        Box& box = g_boxes[i];
        if (wx >= box.x && wx <= box.x + box.w && wy >= box.y && wy <= box.y + box.h)
            return i;
    }
    return -1;
}

void Canvas_ClearSelection() {
    for (auto& box : g_boxes) box.selected = false;
}

// 캔버스 메시지 프로시저
LRESULT CALLBACK CanvasProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_LBUTTONDBLCLK: {
        int x = screenToWorldX(GET_X_LPARAM(lParam));
        int y = screenToWorldY(GET_Y_LPARAM(lParam));
        Canvas_AddBox(x, y, nullptr);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int wx = screenToWorldX(GET_X_LPARAM(lParam));
        int wy = screenToWorldY(GET_Y_LPARAM(lParam));
        g_dragging = Canvas_GetBoxIndexAt(wx, wy);
        if (g_dragging != -1) {
            Canvas_ClearSelection();
            g_boxes[g_dragging].selected = true;
            g_dragOffsetX = wx - g_boxes[g_dragging].x;
            g_dragOffsetY = wy - g_boxes[g_dragging].y;
            InvalidateRect(hwnd, NULL, TRUE);
            SetCapture(hwnd);
        }
        return 0;
    }
    case WM_MOUSEMOVE: {
        if (g_panning) {
            int dx = GET_X_LPARAM(lParam) - g_panStartX;
            int dy = GET_Y_LPARAM(lParam) - g_panStartY;
            g_offsetX = g_viewStartX + dx;
            g_offsetY = g_viewStartY + dy;
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        if (g_dragging != -1 && (wParam & MK_LBUTTON)) {
            int wx = screenToWorldX(GET_X_LPARAM(lParam));
            int wy = screenToWorldY(GET_Y_LPARAM(lParam));
            Box& box = g_boxes[g_dragging];
            box.x = wx - g_dragOffsetX;
            box.y = wy - g_dragOffsetY;
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        return 0;
    }
    case WM_LBUTTONUP: {
        if (g_dragging != -1) {
            g_dragging = -1;
            ReleaseCapture();
        }
        return 0;
    }
    case WM_RBUTTONDOWN: {
        g_panning = true;
        g_panStartX = GET_X_LPARAM(lParam);
        g_panStartY = GET_Y_LPARAM(lParam);
        g_viewStartX = g_offsetX;
        g_viewStartY = g_offsetY;
        SetCapture(hwnd);
        return 0;
    }
    case WM_RBUTTONUP: {
        if (g_panning) {
            g_panning = false;
            ReleaseCapture();
        }
        return 0;
    }
    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        int mx = GET_X_LPARAM(lParam), my = GET_Y_LPARAM(lParam);
        POINT pt = { mx, my };
        ScreenToClient(hwnd, &pt);
        mx = pt.x; my = pt.y;
        double zoom = g_zoom * (delta > 0 ? 1.1 : 1.0 / 1.1);
        Canvas_ZoomTo(mx, my, zoom);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        for (const auto& box : g_boxes) {
            int sx = worldToScreenX(box.x), sy = worldToScreenY(box.y);
            int sw = int(box.w * g_zoom), sh = int(box.h * g_zoom);
            HBRUSH brush = CreateSolidBrush(box.selected ? RGB(70,130,255) : RGB(186,225,255));
            HGDIOBJ oldBrush = SelectObject(hdc, brush);
            Rectangle(hdc, sx, sy, sx + sw, sy + sh);
            SelectObject(hdc, oldBrush);
            DeleteObject(brush);
            RECT r = { sx+8, sy+8, sx+sw-8, sy+sh-8 };
            SetBkMode(hdc, TRANSPARENT);
            DrawText(hdc, L"Area/Node", -1, &r, DT_SINGLELINE | DT_LEFT | DT_TOP);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
