#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/client.h"
#endif

#include "innerstat/client/shape.h"
#include "innerstat/client/canvas.h"
#include "innerstat/client/port.h"
#include "innerstat/client/dialog.h"

#include <wx/textdlg.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <sstream>
#include <format>
#include <stdio.h>
#include <string>

INNERSTAT_BEGIN_NAMESPACE

Shape::Shape(int x, int y, int w, int h, 
    MainCanvas* c, Shape* p, const std::string& l,
    const ShapeType& t)
    : rect(wxPoint(x,y), wxSize(w, h)), canvas(c), parent(p), label(l),
        scale(&(canvas->scale)), offset(&(canvas->offset)), type(t) {
    SetPortCount(1);
}

Shape::~Shape(){
    for(Shape* area : childAreas){
        delete area;
    }
    childAreas.clear();
}

void Shape::Draw(wxDC& dc) const {
    wxFont oldFont = dc.GetFont();
    
    wxRect screenRect(GetScreenRect());
    int w = (int)(rect.width * (*scale));
    int h = (int)(rect.height * (*scale));
    
    dc.SetBrush(isSelected ? wxColour(70,130,255) : wxColour(186, 225, 255));
    dc.SetPen(wxPen(*wxBLACK, std::max(1, (int)(2 * (*scale)))));
    dc.DrawRoundedRectangle(screenRect, (int)(10 * (*scale)));
    
    wxFont font((int)(9 * (*scale)), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(font);
    dc.DrawText(label, screenRect.GetPosition() + wxPoint((int)(5 * (*scale)), (int)(5 * (*scale)) + h));
    dc.SetFont(oldFont);
    
    for (const Port& port : ports) {
        port.Draw(dc, screenRect);
    }
    
    if(isSelected){
        dc.SetPen(wxPen(*wxBLACK, 1));
        const int handleScale = 6;
        wxRect handle(screenRect.GetBottomRight() .x - handleScale / 2, screenRect.GetBottomRight() .y - handleScale / 2, handleScale, handleScale);
        dc.DrawRectangle(handle);
    }
}

void Shape::AddChildArea(Shape* area) {
    childAreas.push_back(area);
    canvas->UpdateAllShapesList();
    canvas->RefreshTree();
    canvas->Refresh();
}

void Shape::OpenPropertyDialog() {
    AreaProperties* ret = ShowAreaPropertyDialog(canvas, this);

    if(ret == nullptr) return;

    this->label = ret->label;
    type = ret->areaType;
    SetPortCount(ret->portCount);
    delete ret;

    canvas->RefreshTree();
    canvas->Refresh();
}

void Shape::OpenAddShapeDialog() {
    AreaProperties* ret = ShowAddAreaDialog(canvas, 1);

    if(ret == nullptr) return;

    int offsetX = childAreas.size() * 40;
    int offsetY = 0;
    Shape* newArea = new Shape(offsetX, offsetY, 40, 40, canvas, this, ret->label, ret->areaType);
    this->AddChildArea(newArea);

    delete ret;
}

void Shape::SetPortCount(int count) {
    ports.clear();
    for (int i = 0; i < count; ++i) {
        double ratio = (i + 1.0) / (count + 1.0);
        std::string id = label + "_p" + std::to_string(i);
        ports.emplace_back(canvas, id, wxPoint2DDouble(ratio, 0.0));
    }
}

const Port* Shape::HitTestPort(const wxPoint& pos, const Shape** outShape) const {
    const std::vector<Port>& ports = this->GetPorts();
    for (const Port& port : ports) {
        wxPoint screenPos = port.GetScreenPosition(GetScreenRect());
        wxRect hitbox(screenPos.x - 6, screenPos.y - 6, 12, 12);
        if (hitbox.Contains(pos)) {
            if (outShape) *outShape = this;
            return &port;
        }
    }
    return nullptr;
}

ShapeHandle Shape::HitTestShape(wxPoint& mouse) {
    const int hs = 6; // 핸들 크기 (6x6)

    wxRect scaledRect(GetScreenRect());

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


std::string Shape::Serialize() const {
    // TODO
    return nullptr;
}

Shape* Shape::Deserialize(const std::string& line, MainCanvas* canvas) {
    // TODO
    return nullptr;
}

INNERSTAT_END_NAMESPACE