#include "Shape.h"

Shape::Shape(double x, double y, double w, double h)
    : pos(x, y), width(w), height(h) { }

bool Shape::Contains(const wxPoint& screenPt, double scale, const wxPoint2DDouble& offset) const {
    wxPoint sp(pos.m_x * scale + offset.m_x, pos.m_y * scale + offset.m_y);
    int w = width * scale;
    int h = height * scale;
    return wxRect(sp.x, sp.y, w, h).Contains(screenPt);
}

HandleType Shape::HitTestHandle(const wxPoint& mouse, double scale, const wxPoint2DDouble& offset) const {
    const int hs = 6; // 핸들 크기 (6x6)

    wxPoint sp(pos.m_x * scale + offset.m_x, pos.m_y * scale + offset.m_y);
    int w = width * scale;
    int h = height * scale;

    wxPoint handles[8] = {
        {sp.x, sp.y},                   // TopLeft
        {sp.x + w / 2, sp.y},           // Top
        {sp.x + w, sp.y},               // TopRight
        {sp.x, sp.y + h / 2},           // Left
        {sp.x + w, sp.y + h / 2},       // Right
        {sp.x, sp.y + h},               // BottomLeft
        {sp.x + w / 2, sp.y + h},       // Bottom
        {sp.x + w, sp.y + h}            // BottomRight
    };

    for (int i = 0; i < 8; ++i) {
        wxRect handle(handles[i].x - hs / 2, handles[i].y - hs / 2, hs, hs);
        if (handle.Contains(mouse))
            return static_cast<HandleType>(i);
    }

    return HandleType::None;
}
