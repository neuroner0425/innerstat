#include "Shape.h"
#include "Area.h"
#include "Canvas.h"

Shape::Shape(double x, double y, double w, double h, MainCanvas* c, Area* p, const std::string& l)
    : pos(x, y), width(w), height(h), canvas(c), parent(p), label(l) { }

bool Shape::Contains(const wxPoint& screenPt) const {
    wxPoint sp(pos.m_x * canvas->scale + canvas->offset.m_x, pos.m_y * canvas->scale + canvas->offset.m_y);
    int w = width * canvas->scale;
    int h = height * canvas->scale;
    return wxRect(sp.x, sp.y, w, h).Contains(screenPt);
}

HandleType Shape::HitTestHandle(const wxPoint& mouse) const {
    const int hs = 6; // 핸들 크기 (6x6)

    wxPoint sp(pos.m_x * canvas->scale + canvas->offset.m_x, pos.m_y * canvas->scale + canvas->offset.m_y);
    int w = width * canvas->scale;
    int h = height * canvas->scale;

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
        wxPoint screenPos = port.GetScreenPosition(this->pos, this->width, this->height, canvas->scale, canvas->offset);
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
    // if (activeHandle != HandleType::None) {
    //     canvas->ResizingShape(this, activeHandle);
    //     return true;
    // }

    if (Contains(mouse)) {
        canvas->DraggingShape(this);
        return true;
    }
    return false;
}

bool Shape::OpenProperty(wxPoint& pos){
    if (Contains(pos)) {
        OpenPropertyDialog(canvas);
        canvas->UpdateAllShapesList();
        canvas->Refresh();
        return true;
    }
    return false;
}