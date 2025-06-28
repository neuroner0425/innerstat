#pragma once
#include <windows.h>
#include <commctrl.h>
#include <vector>

// Box 구조체
struct Box {
    int x, y, w, h;
    bool selected;
    HTREEITEM treeItem;
};

// 전역 상태 (간단화, 필요시 클래스/싱글톤화 가능)
extern std::vector<Box> g_boxes;
extern double g_zoom;
extern int g_offsetX, g_offsetY;

// 함수 선언
LRESULT CALLBACK CanvasProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void Canvas_AddBox(int x, int y, HTREEITEM treeItem);
void Canvas_ZoomTo(int mx, int my, double newZoom);
void Canvas_SelectBoxByTreeItem(HTREEITEM treeItem);
int  Canvas_GetBoxIndexAt(int wx, int wy);
void Canvas_ClearSelection();
