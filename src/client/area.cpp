#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/base.h"
#endif

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

INNERSTAT_BEGIN_NAMESPACE

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
            int offsetX = (childAreas.size() + childNodes.size()) * 40;
            int offsetY = 0;
            Area* newArea = new Area(offsetX, offsetY, 40, 40, canvas, this, areaCast->label, areaCast->areaType);
            this->AddChildArea(newArea);
    }else{
            NodeProperties* nodeCast = dynamic_cast<NodeProperties*>(ret);
            int offsetX = (childAreas.size() + childNodes.size()) * 40;
            int offsetY = 0;
            Node* newNode = new Node(offsetX, offsetY, 40, 40, canvas, this, nodeCast->label);
            this->AddChildNode(newNode);
    }
}


void Area::Draw(wxDC& dc) const {
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


std::string Area::Serialize() const {
    // TODO
    return nullptr;
}

Area* Area::Deserialize(const std::string& line, MainCanvas* canvas) {
    // TODO
    return nullptr;
}


void Area::AddChildArea(Area* area) {
    childAreas.push_back(area);
    canvas->UpdateAllShapesList();
    canvas->RefreshTree();
    canvas->Refresh();
}

void Area::AddChildNode(Node* node) {
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

INNERSTAT_END_NAMESPACE