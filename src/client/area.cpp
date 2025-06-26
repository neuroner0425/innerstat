#include "innerstat/client/area.h"
#include "innerstat/client/node.h"
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

Area::Area(int x, int y, int w, int h, 
    MainCanvas* c, Area* p, const std::string& l,
    const AreaType& t)
    : Shape(x, y, w, h, c, p, l), type(t) {
    SetPortCount(1);
}

Area::~Area(){
    for(Area* area : childAreas){
        delete area;
    }
    childAreas.clear();

    for(Node* node : childNodes){
        delete node;
    }
    childNodes.clear();
}

void Area::SetPortCount(int count) {
    ports.clear();
    for (int i = 0; i < count; ++i) {
        double ratio = (i + 1.0) / (count + 1.0);
        std::string id = label + "_p" + std::to_string(i);
        ports.emplace_back(canvas, id, wxPoint2DDouble(ratio, 0.0));
    }
}

void Area::OpenPropertyDialog() {
    AreaProperties* ret = ShowAreaPropertyDialog(canvas, this);

    if(ret == nullptr) return;

    this->label = ret->label;
    type = ret->areaType;
    SetPortCount(ret->portCount);
    delete ret;

    canvas->RefreshTree();
    canvas->Refresh();
}

void Area::OpenAddShapeDialog() {
    ShapeProperties* ret = ShowAddShapeDialog(canvas);

    if(ret == nullptr) return;

    AreaProperties* areaCast = dynamic_cast<AreaProperties*>(ret);
    if(areaCast){
            int offsetX = position.x + 40 + childAreas.size() * 40;
            int offsetY = position.y + 40;
            Area* newArea = new Area(offsetX, offsetY, 120, 120, canvas, this, areaCast->label, areaCast->areaType);
            this->AddSubArea(newArea);
    }else{
            int offsetX = position.x + 40 + childNodes.size() * 40;
            int offsetY = position.y + 100;
            Node* newNode = new Node(offsetX, offsetY, 120, 120, canvas, this, areaCast->label);
            this->AddNode(newNode);
    }
}


void Area::Draw(wxDC& dc) const {
    wxFont oldFont = dc.GetFont();
    double s = canvas->scale;

    wxPoint screenPos(position.x * canvas->scale + canvas->offset.x, position.y * canvas->scale + canvas->offset.y);
    int w = (int)(position.width * s);
    int h = (int)(position.height * s);

    dc.SetBrush(isSelected ? wxColour(70,130,255) : wxColour(186, 225, 255));
    dc.SetPen(wxPen(*wxBLACK, std::max(1, (int)(2 * s))));
    dc.DrawRoundedRectangle(screenPos.x, screenPos.y, w, h, (int)(10 * s));
    
    wxFont font((int)(9 * s), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(font);
    dc.DrawText(label, screenPos + wxPoint((int)(5 * s), (int)(5 * s) + h));
    dc.SetFont(oldFont);

    for (const Port& port : ports) {
        wxPoint p = port.GetScreenPosition(position.GetPosition(), position.width, position.height, s, canvas->offset);
        port.Draw(dc, p);
    }
    
    if(isSelected){
        dc.SetPen(wxPen(*wxBLACK, 1));
        const int hs = 6;
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
            dc.DrawRectangle(handle);
        }
    }
}


std::string Area::Serialize() const {
    // TODO
    return nullptr;
}

Area* Area::Deserialize(const std::string& line, MainCanvas* canvas) {
    // TODO
    return nullptr;
}


void Area::AddSubArea(Area* area) {
    childAreas.push_back(area);
    canvas->UpdateAllShapesList();
    canvas->RefreshTree();
    canvas->Refresh();
}

void Area::AddNode(Node* node) {
    childNodes.push_back(node);
    canvas->UpdateAllShapesList();
    canvas->RefreshTree();
    canvas->Refresh();
}

const Port* Area::HitTestPort(const wxPoint& pos, const Shape** outShape) const {
    for (std::vector<Area*>::const_reverse_iterator it = childAreas.rbegin(); it != childAreas.rend(); ++it)
        if(const Port* childPort = (*it)->HitTestPort(pos, outShape)) return childPort;

    for (std::vector<Node*>::const_reverse_iterator it = childNodes.rbegin(); it != childNodes.rend(); ++it)
        if(const Port* childPort = (*it)->HitTestPort(pos, outShape)) return childPort;

    return Shape::HitTestPort(pos, outShape);
}

ShapeHandle Area::HitTestShape(wxPoint& mouse) {
    for (std::vector<Area*>::const_reverse_iterator it = childAreas.rbegin(); it != childAreas.rend(); ++it){
        ShapeHandle ret = (*it)->HitTestShape(mouse);
        if(ret.shape != nullptr) return ret;
    }

    for (std::vector<Node*>::const_reverse_iterator it = childNodes.rbegin(); it != childNodes.rend(); ++it){
        ShapeHandle ret = (*it)->HitTestShape(mouse);
        if(ret.shape != nullptr) return ret;
    }

    return Shape::HitTestShape(mouse);
}
