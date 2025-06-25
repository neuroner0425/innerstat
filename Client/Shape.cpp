#include "Shape.h"
#include "Area.h"
#include "Canvas.h"

Shape::Shape(int x, int y, int w, int h, MainCanvas* c, Area* p, const std::string& l)
    : position(wxPoint(x,y), wxSize(w, h)), canvas(c), parent(p), label(l) { }

bool Shape::Contains(const wxPoint& screenPt) const {
    wxRect scaledRect(
        position.x * canvas->scale + canvas->offset.x,
        position.y * canvas->scale + canvas->offset.y,
        position.width * canvas->scale,
        position.height * canvas->scale
    );
    return scaledRect.Contains(screenPt);
}

const Port* Shape::HitTestPort(const wxPoint& pos, const Shape** outShape) const{
    const std::vector<Port>& ports = this->GetPorts();
    for (const Port& port : ports) {
        wxPoint screenPos = port.GetScreenPosition(position.GetPosition(), position.width, position.height, canvas->scale, canvas->offset);
        wxRect hitbox(screenPos.x - 6, screenPos.y - 6, 12, 12);
        if (hitbox.Contains(pos)) {
            if (outShape) *outShape = this;
            return &port;
        }
    }
    return nullptr;
}

ShapeHandle Shape::HitTestShape(wxPoint& mouse){
    const int hs = 6; // 핸들 크기 (6x6)

    // 스케일 및 오프셋을 적용한 사각형
    wxRect scaledRect(
        position.x * canvas->scale + canvas->offset.x,
        position.y * canvas->scale + canvas->offset.y,
        position.width * canvas->scale,
        position.height * canvas->scale
    );

    wxPoint handles[8] = {
        scaledRect.GetTopLeft(),                                                            // TopLeft
        wxPoint(scaledRect.GetLeft() + scaledRect.GetWidth() / 2, scaledRect.GetTop()),     // Top
        scaledRect.GetTopRight(),                                                           // TopRight
        wxPoint(scaledRect.GetLeft(), scaledRect.GetTop() + scaledRect.GetHeight() / 2),    // Left
        wxPoint(scaledRect.GetRight(), scaledRect.GetTop() + scaledRect.GetHeight() / 2),   // Right
        scaledRect.GetBottomLeft(),                                                         // BottomLeft
        wxPoint(scaledRect.GetLeft() + scaledRect.GetWidth() / 2, scaledRect.GetBottom()),  // Bottom
        scaledRect.GetBottomRight()                                                         // BottomRight
    };

    for (int i = 0; i < 8; ++i) {
        wxRect handle(handles[i].x - hs / 2, handles[i].y - hs / 2, hs, hs);
        if (handle.Contains(mouse))
            return ShapeHandle(this, static_cast<HandleType>(i));
    }

    if (scaledRect.Contains(mouse))
        return ShapeHandle(this, HandleType::Body);

    return ShapeHandle(nullptr, HandleType::None);
}