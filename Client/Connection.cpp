#include "Connection.h"
#include "Shape.h"

void Connection::Draw(wxDC& dc, double scale, const wxPoint2DDouble& offset) const {
    if (!from || !to)
        return;

    // 시작/도착 포트의 실제 화면 위치 계산
    wxPoint a = from->GetScreenPosition(fromShape->pos, fromShape->width, fromShape->height, scale, offset);
    wxPoint b = to->GetScreenPosition(toShape->pos, toShape->width, toShape->height, scale, offset);

    dc.SetPen(wxPen(*wxBLACK, 2));

    // 꺾인 라인(엘보 형태) - 수평 → 수직 → 수평
    int midX = a.x;
    int midY = b.y;

    dc.DrawLine(a.x, a.y, midX, a.y);   // 수평
    dc.DrawLine(midX, a.y, midX, midY); // 수직
    dc.DrawLine(midX, midY, b.x, b.y);  // 수평
}
