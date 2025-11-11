#include "innerstat/client/port.h"
#include "innerstat/client/canvas.h"

INNERSTAT_BEGIN_NAMESPACE

Port::Port(MainCanvas* canvas, const std::string& id, const wxPoint2DDouble& relPos)
    : canvas(canvas), id(id), relativePos(relPos) {}

wxPoint Port::GetScreenPosition(const wxRect& rect) const{
    return wxPoint(
        rect.GetX() + int(rect.GetWidth() * relativePos.m_x),
        rect.GetY() + int(rect.GetHeight() * relativePos.m_y)
    );
}

void Port::Draw(wxGraphicsContext& gc, const wxRect& rect) const{
    double s = canvas->scale;
    int width = (int)(8 * s);
    gc.SetBrush(wxBrush(wxColour(100, 100, 255)));
    gc.SetPen(wxPen(*wxBLACK, std::max(1, (int)(1 * s))));
    wxPoint pos = GetScreenPosition(rect);
    gc.DrawRectangle(pos.x - width / 2, pos.y - width / 2, width, width);
}

INNERSTAT_END_NAMESPACE