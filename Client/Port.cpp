#include "Port.h"
#include "Canvas.h"

Port::Port(MainCanvas* canvas, const std::string& id, const wxPoint2DDouble& relPos)
    : canvas(canvas), id(id), relativePos(relPos) {}

wxPoint Port::GetScreenPosition(const wxPoint2DDouble& shapePos, double width, double height, double scale, const wxPoint2DDouble& offset) const {
    double x = shapePos.m_x + relativePos.m_x * width;
    double y = shapePos.m_y + relativePos.m_y * height;
    cachedScreenPos = wxPoint((int)(x * scale + offset.m_x), (int)(y * scale + offset.m_y));
    return cachedScreenPos;
}

void Port::Draw(wxDC& dc, const wxPoint& screenPos) const {
    double s = canvas->scale;
    int width = (int)(8 * s);
    dc.SetBrush(wxBrush(wxColour(100, 100, 255)));
    dc.SetPen(wxPen(*wxBLACK, std::max(1, (int)(1 * s))));
    dc.DrawRectangle(screenPos - wxPoint(width / 2, width / 2), wxSize(width, width));
}