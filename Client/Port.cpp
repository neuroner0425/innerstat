#include "Port.h"

Port::Port(const std::string& id, const wxPoint2DDouble& relPos)
    : id(id), relativePos(relPos) {}

wxPoint Port::GetScreenPosition(const wxPoint2DDouble& shapePos, double width, double height, double scale, const wxPoint2DDouble& offset) const {
    double x = shapePos.m_x + relativePos.m_x * width;
    double y = shapePos.m_y + relativePos.m_y * height;
    cachedScreenPos = wxPoint(x * scale + offset.m_x, y * scale + offset.m_y);
    return cachedScreenPos;
}

void Port::Draw(wxDC& dc, const wxPoint& screenPos) const {
    dc.SetBrush(wxBrush(wxColour(100, 100, 255)));
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawCircle(screenPos, 4);  // 반지름 4픽셀 원
}
