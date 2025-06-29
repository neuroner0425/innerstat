#include "innerstat/client/port.h"
#include "innerstat/client/canvas.h"

INNERSTAT_BEGIN_NAMESPACE

Port::Port(MainCanvas* canvas, const std::string& id, const wxPoint2DDouble& relPos)
    : canvas(canvas), id(id), relativePos(relPos) {}

wxPoint Port::GetScreenPosition(const wxRect& rect) const{
    wxPoint screenPos(rect.GetPosition());
    screenPos.x += rect.GetWidth() / 2;
    return screenPos;
}

void Port::Draw(wxDC& dc, const wxRect& rect) const{
    double s = canvas->scale;
    int width = (int)(8 * s);
    dc.SetBrush(wxBrush(wxColour(100, 100, 255)));
    dc.SetPen(wxPen(*wxBLACK, std::max(1, (int)(1 * s))));
    dc.DrawRectangle(GetScreenPosition(rect) - wxPoint(width / 2, width / 2), wxSize(width, width));
}

INNERSTAT_END_NAMESPACE