#include "Shape.h"

Shape::Shape(double x, double y, double w, double h)
    : pos(x, y), width(w), height(h) {}

bool Shape::Contains(const wxPoint& screenPt, double scale, const wxPoint2DDouble& offset) const {
    wxPoint sp(pos.m_x * scale + offset.m_x, pos.m_y * scale + offset.m_y);
    int w = width * scale;
    int h = height * scale;
    return wxRect(sp.x, sp.y, w, h).Contains(screenPt);
}

HandleType Shape::HitTestHandle(const wxPoint& mouse, double scale, const wxPoint2DDouble& offset) const {
    const int hs = 6;
    wxPoint sp(pos.m_x * scale + offset.m_x, pos.m_y * scale + offset.m_y);
    int w = width * scale;
    int h = height * scale;
    wxPoint handles[8] = {
        {sp.x, sp.y}, {sp.x + w/2, sp.y}, {sp.x + w, sp.y},
        {sp.x, sp.y + h/2}, {sp.x + w, sp.y + h/2},
        {sp.x, sp.y + h}, {sp.x + w/2, sp.y + h}, {sp.x + w, sp.y + h}
    };
    for (int i = 0; i < 8; ++i) {
        wxRect handle(handles[i].x - hs/2, handles[i].y - hs/2, hs, hs);
        if (handle.Contains(mouse))
            return static_cast<HandleType>(i);
    }
    return HandleType::None;
}