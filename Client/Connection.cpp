#include "Connection.h"
#include "Shape.h"

void Connection::Draw(wxDC& dc, double scale, const wxPoint2DDouble& offset) const {
    if (!from || !to)
        return;

    // 시작/도착 포트의 실제 화면 위치 계산
    wxPoint a = from->GetScreenPosition(fromShape->pos, fromShape->width, fromShape->height, scale, offset);
    wxPoint b = to->GetScreenPosition(toShape->pos, toShape->width, toShape->height, scale, offset);

    dc.SetPen(wxPen(*wxBLACK, 2));

    int minimumLineLength = 50 * scale;
    int midY = (a.y > b.y)? b.y - minimumLineLength : a.y - minimumLineLength;

    dc.DrawLine(a.x, a.y, a.x, a.y - minimumLineLength);
    dc.DrawLine(a.x, a.y - minimumLineLength , a.x, midY ); // 수직
    dc.DrawLine(a.x, midY, b.x, midY);   // 수평
    dc.DrawLine(b.x, b.y - minimumLineLength, b.x, midY);  // 수직
    dc.DrawLine(b.x, b.y, b.x, b.y - minimumLineLength);
}
