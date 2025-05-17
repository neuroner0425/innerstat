#include "Shape.h"
#include "Canvas.h"

Shape::Shape(double x, double y, double w, double h, MyCanvas* p)
    : pos(x, y), width(w), height(h), parent(p) { }

bool Shape::Contains(const wxPoint& screenPt) const {
    wxPoint sp(pos.m_x * parent->scale + parent->offset.m_x, pos.m_y * parent->scale + parent->offset.m_y);
    int w = width * parent->scale;
    int h = height * parent->scale;
    return wxRect(sp.x, sp.y, w, h).Contains(screenPt);
}

HandleType Shape::HitTestHandle(const wxPoint& mouse) const {
    const int hs = 6; // 핸들 크기 (6x6)

    wxPoint sp(pos.m_x * parent->scale + parent->offset.m_x, pos.m_y * parent->scale + parent->offset.m_y);
    int w = width * parent->scale;
    int h = height * parent->scale;

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


const Port* Shape::HitTestPort(const wxPoint& pos, const Shape** outShape) const{
    const std::vector<Port>& ports = this->GetPorts();
    for (const Port& port : ports) {
        wxPoint screenPos = port.GetScreenPosition(this->pos, this->width, this->height, parent->scale, parent->offset);
        wxRect hitbox(screenPos.x - 6, screenPos.y - 6, 12, 12);
        if (hitbox.Contains(pos)) {
            if (outShape) *outShape = this;
            return &port;
        }
    }
    return nullptr;
}

bool Shape::HitTestShape(wxPoint& mouse){
    HandleType activeHandle = this->HitTestHandle(mouse);
    if (activeHandle != HandleType::None) {
        parent->ResizingShape(this, activeHandle);
        return true;
    }

    if (Contains(mouse)) {
        parent->DraggingShape(this);
        return true;
    }
    return false;
}

bool Shape::OpenProperty(wxPoint& pos){
    if (Contains(pos)) {
        OpenPropertyDialog(parent);
        parent->Refresh();
        return true;
    }
    return false;
}