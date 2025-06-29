#include "innerstat/client/connection.h"
#include "innerstat/client/shape.h"

INNERSTAT_BEGIN_NAMESPACE

void Connection::Draw(wxDC& dc, double scale, const wxPoint2DDouble& offset) const {
    if (!from || !to)
        return;

    wxPoint a = from->GetScreenPosition(fromShape->GetScreenRect());
    wxPoint b = to->GetScreenPosition(toShape->GetScreenRect());

    dc.SetPen(wxPen(*wxBLUE, (int)(2 * scale)));

    int minimumLineLength = (int)(50 * scale);
    int midY = (a.y > b.y)? b.y - minimumLineLength : a.y - minimumLineLength;

    dc.DrawLine(a.x, a.y, a.x, a.y - minimumLineLength);
    dc.DrawLine(a.x, a.y - minimumLineLength, a.x, midY);
    dc.DrawLine(a.x, midY, b.x, midY);
    dc.DrawLine(b.x, b.y - minimumLineLength, b.x, midY);
    dc.DrawLine(b.x, b.y, b.x, b.y - minimumLineLength);
}

INNERSTAT_END_NAMESPACE