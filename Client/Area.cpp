#include "Area.h"
#include "Node.h"
#include "Canvas.h"
#include "Port.h"
#include <wx/textdlg.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <sstream>
#include <format>
#include <stdio.h>
#include <string>

Area::Area(double x, double y, double w, double h, 
    MainCanvas* c, Area* p, const std::string& l,
    const AreaType& t)
    : Shape(x, y, w, h, c, p, l), type(t) {
    SetPortCount(1);
}

Area::~Area(){
    for(Area* area : subAreas){
        delete area;
    }
    subAreas.clear();

    for(Node* node : nodes){
        delete node;
    }
    nodes.clear();
}

void Area::SetPortCount(int count) {
    ports.clear();
    for (int i = 0; i < count; ++i) {
        double ratio = (i + 1.0) / (count + 1.0);
        std::string id = label + "_p" + std::to_string(i);
        ports.emplace_back(canvas, id, wxPoint2DDouble(ratio, 0.0));
    }
}


void Area::OpenPropertyDialog(MainCanvas *canvas) {
    wxDialog dlg(canvas, wxID_ANY, "Area Properties");
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    wxArrayString types = { "Other", "OS", "VM", "Container", "Network" };

    wxTextCtrl *labelctrl = new wxTextCtrl(&dlg, wxID_ANY, wxString(label), wxDefaultPosition, wxDefaultSize);
    wxChoice *typeCtrl = new wxChoice(&dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, types);
    wxSpinCtrl *portCount = new wxSpinCtrl(&dlg, wxID_ANY);
    wxButton *addShapeBtn = new wxButton(&dlg, wxID_ANY, "Add Shape");

    typeCtrl->SetSelection(getTypeInt());
    portCount->SetRange(0, 10);
    portCount->SetValue(ports.size());

    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "Label:"), 0, wxALL, 5);
    sizer->Add(labelctrl, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "Type:"), 0, wxALL, 5);
    sizer->Add(typeCtrl, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "Port Count:"), 0, wxALL, 5);
    sizer->Add(portCount, 0, wxEXPAND | wxALL, 5);
    sizer->Add(addShapeBtn, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxButton(&dlg, wxID_OK), 0, wxALIGN_CENTER | wxALL, 10);

    dlg.SetSizerAndFit(sizer);
    dlg.SetSize(wxSize(200, -1));

    addShapeBtn->Bind(wxEVT_BUTTON,[=](wxCommandEvent &evt){
        this->OpenAddShapeDialog(canvas);
    });

    if (dlg.ShowModal() == wxID_OK) {
        this->label = labelctrl->GetValue().ToStdString();
        type = getTypeByInt(typeCtrl->GetSelection());
        SetPortCount(portCount->GetValue());
        canvas->RefreshTree();
        canvas->Refresh();
    }
}

void Area::OpenAddShapeDialog(MainCanvas* canvas) {
    wxDialog dlg(canvas, wxID_ANY, "Add Shape");
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* midSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);

    wxArrayString shapeTypes = { "Area", "Node" };
    wxArrayString types = { "Other", "OS", "VM", "Container", "Network" };

    wxChoice* shapeTypeCtrl = new wxChoice(&dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, shapeTypes);
    shapeTypeCtrl->SetSelection(0);

    wxTextCtrl* labelCtrl = nullptr;
    wxChoice* typeCtrl = nullptr;

    topSizer->Add(new wxStaticText(&dlg, wxID_ANY, "Shape Type:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    topSizer->Add(shapeTypeCtrl, 0, wxALL, 10);

    bottomSizer->Add(new wxButton(&dlg, wxID_OK), 0, wxALL, 10);
    bottomSizer->Add(new wxButton(&dlg, wxID_CANCEL), 0, wxALL, 10);

    sizer->Add(topSizer, 0, wxEXPAND);
    sizer->Add(midSizer, 1, wxEXPAND | wxALL, 10);
    sizer->Add(bottomSizer, 0, wxALIGN_CENTER);

    dlg.SetSizerAndFit(sizer);
    dlg.SetSize(wxSize(200, -1));

    auto updateMidSizer = [&]() {
        midSizer->Clear(true);

        midSizer->Add(new wxStaticText(&dlg, wxID_ANY, "Label:"), 0, wxALL, 5);
        labelCtrl = new wxTextCtrl(&dlg, wxID_ANY, wxString("New Node"));
        
        midSizer->Add(labelCtrl, 0, wxEXPAND | wxALL, 5);

        if (shapeTypeCtrl->GetSelection() == 0) {
            labelCtrl->SetValue(wxString("New Area"));
            midSizer->Add(new wxStaticText(&dlg, wxID_ANY, "Type:"), 0, wxALL, 5);
            typeCtrl = new wxChoice(&dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, types);
            typeCtrl->SetSelection(0);
            midSizer->Add(typeCtrl, 0, wxEXPAND | wxALL, 5);
        } else {
            typeCtrl = nullptr;
        }

        dlg.Layout();
        dlg.Fit();
    };

    updateMidSizer();

    shapeTypeCtrl->Bind(wxEVT_CHOICE, [&](wxCommandEvent&) {
        updateMidSizer();
    });

    int status = dlg.ShowModal();
    if (status == wxID_OK) {
        std::string label = labelCtrl->GetValue().ToStdString();

        if (shapeTypeCtrl->GetSelection() == 0) {
            double offsetX = this->pos.m_x + 30 + this->GetSubAreas().size() * 20;
            double offsetY = this->pos.m_y + 40;

            AreaType areaType = getTypeByInt(typeCtrl->GetSelection());

            Area* newArea = new Area(offsetX, offsetY, 150, 80, canvas, this, label, areaType);
            this->AddSubArea(newArea);
        } else {
            double offsetX = this->pos.m_x + 30 + this->GetNodes().size() * 20;
            double offsetY = this->pos.m_y + 100;

            Node* newNode = new Node(offsetX, offsetY, 150, 80, canvas, this, label);
            this->AddNode(newNode);
        }
    }
}


void Area::Draw(wxDC& dc) const {
    wxFont oldFont = dc.GetFont();
    double s = canvas->scale;

    wxPoint screenPos((int)(pos.m_x * s + canvas->offset.m_x), (int)(pos.m_y * s + canvas->offset.m_y));
    int w = (int)(width * s);
    int h = (int)(height * s);

    dc.SetBrush(selected ? wxColour(70,130,255) : wxColour(186, 225, 255));
    dc.SetPen(wxPen(*wxBLACK, std::max(1, (int)(2 * s))));
    dc.DrawRoundedRectangle(screenPos.x, screenPos.y, w, h, (int)(10 * s));
    
    wxFont font((int)(9 * s), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(font);
    dc.DrawText(label, screenPos + wxPoint((int)(5 * s), (int)(5 * s) + h));
    dc.SetFont(oldFont);

    for (const Port& port : ports) {
        wxPoint p = port.GetScreenPosition(pos, width, height, s, canvas->offset);
        port.Draw(dc, p);
    }
    
    // if(selected){
    //     dc.SetPen(wxPen(*wxBLACK, 1));
    //     const int hs = 6;
    //     wxPoint sp(pos.m_x * canvas->scale + canvas->offset.m_x, pos.m_y * canvas->scale + canvas->offset.m_y);
    //     int w = width * canvas->scale;
    //     int h = height * canvas->scale;

    //     wxPoint handles[8] = {
    //         {sp.x, sp.y},                   // TopLeft
    //         {sp.x + w / 2, sp.y},           // Top
    //         {sp.x + w, sp.y},               // TopRight
    //         {sp.x, sp.y + h / 2},           // Left
    //         {sp.x + w, sp.y + h / 2},       // Right
    //         {sp.x, sp.y + h},               // BottomLeft
    //         {sp.x + w / 2, sp.y + h},       // Bottom
    //         {sp.x + w, sp.y + h}            // BottomRight
    //     };

    //     for (int i = 0; i < 8; ++i) {
    //         wxRect handle(handles[i].x - hs / 2, handles[i].y - hs / 2, hs, hs);
    //         dc.DrawRectangle(handle);
    //     }
    // }
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
    subAreas.push_back(area);
    canvas->RefreshTree();
    canvas->Refresh();
}

void Area::AddNode(Node* node) {
    nodes.push_back(node);
    canvas->RefreshTree();
    canvas->Refresh();
}

const std::vector<Area*>& Area::GetSubAreas() const{
    return subAreas;
}

const std::vector<Node*>& Area::GetNodes() const{
    return nodes;
}

const Port* Area::HitTestPort(const wxPoint& pos, const Shape** outShape) const {
    for (std::vector<Area*>::const_reverse_iterator it = GetSubAreas().rbegin(); it != GetSubAreas().rend(); ++it)
        if(const Port* childPort = (*it)->HitTestPort(pos, outShape)) return childPort;

    for (std::vector<Node*>::const_reverse_iterator it = GetNodes().rbegin(); it != GetNodes().rend(); ++it)
        if(const Port* childPort = (*it)->HitTestPort(pos, outShape)) return childPort;

    return Shape::HitTestPort(pos, outShape);
}

bool Area::HitTestShape(wxPoint& mouse) {
    for (std::vector<Area*>::const_reverse_iterator it = GetSubAreas().rbegin(); it != GetSubAreas().rend(); ++it)
        if((*it)->HitTestShape(mouse)) return true;

    for (std::vector<Node*>::const_reverse_iterator it = GetNodes().rbegin(); it != GetNodes().rend(); ++it)
        if((*it)->HitTestShape(mouse)) return true;

    return Shape::HitTestShape(mouse);
}

bool Area::OpenProperty(wxPoint& pos){
    for (std::vector<Area*>::const_reverse_iterator it = GetSubAreas().rbegin(); it != GetSubAreas().rend(); ++it)
        if((*it)->OpenProperty(pos)) return true;

    for (std::vector<Node*>::const_reverse_iterator it = GetNodes().rbegin(); it != GetNodes().rend(); ++it)
        if((*it)->OpenProperty(pos)) return true;

    return Shape::OpenProperty(pos);
}