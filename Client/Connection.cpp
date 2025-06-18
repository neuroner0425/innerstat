#include "Connection.h"
#include "Shape.h"

void Connection::Draw(wxDC& dc, double scale, const wxPoint2DDouble& offset) const {
    if (!from || !to)
        return;

    wxPoint a = from->GetScreenPosition(fromShape->pos, fromShape->width, fromShape->height, scale, offset);
    wxPoint b = to->GetScreenPosition(toShape->pos, toShape->width, toShape->height, scale, offset);

    dc.SetPen(wxPen(*wxBLACK, (int)(2 * scale))); // 선 두께도 scale 적용 추천!

    int minimumLineLength = (int)(50 * scale);
    int midY = (a.y > b.y)? b.y - minimumLineLength : a.y - minimumLineLength;

    dc.DrawLine(a.x, a.y, a.x, a.y - minimumLineLength);
    dc.DrawLine(a.x, a.y - minimumLineLength, a.x, midY);
    dc.DrawLine(a.x, midY, b.x, midY);
    dc.DrawLine(b.x, b.y - minimumLineLength, b.x, midY);
    dc.DrawLine(b.x, b.y, b.x, b.y - minimumLineLength);
}
