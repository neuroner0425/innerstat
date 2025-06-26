#include "innerstat/client/node.h"
#include "innerstat/client/area.h"
#include "innerstat/client/canvas.h"
#include "innerstat/client/port.h"
#include <wx/textdlg.h>
#include <wx/spinctrl.h>
#include <sstream>

Node::Node(int x, int y, int w, int h, 
    MainCanvas* c, Area* p, const std::string& l)
    : Shape(x, y, w, h, c, p, l), active(true), overloaded(false){
    SetPortCount(1);
}

void Node::SetPortCount(int count) {
    ports.clear();
    for (int i = 0; i < count; ++i) {
        double ratio = (i + 1.0) / (count + 1.0);
        std::string id = label + "_p" + std::to_string(i);
        ports.emplace_back(canvas, id, wxPoint2DDouble(ratio, 0.0));
    }
}

void Node::Draw(wxDC &dc) const {
    wxFont oldFont = dc.GetFont();
    double s = canvas->scale;

    wxPoint screenPos(position.x * canvas->scale + canvas->offset.x, position.y * canvas->scale + canvas->offset.y);
    int w = (int)(position.width * s);
    int h = (int)(position.height * s);

    wxColour fill = overloaded ? wxColour(255, 100, 100)
                  : (active ? wxColour(180, 255, 180) : wxColour(200, 200, 200));

    dc.SetBrush(isSelected ? wxBrush(*wxYELLOW) : wxBrush(fill));
    dc.SetPen(wxPen(*wxBLACK, std::max(1, (int)(2 * s))));
    dc.DrawRoundedRectangle(screenPos.x, screenPos.y, w, h, (int)(6 * s));

    wxFont font((int)(9 * s), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(font);
    dc.DrawText(label, screenPos + wxPoint((int)(5 * s), (int)(5 * s) + h));
    dc.SetFont(oldFont);

    for (const auto& port : ports) {
        wxPoint p = port.GetScreenPosition(position.GetPosition(), position.width, position.height, s, canvas->offset);
        port.Draw(dc, p);
    }
    
    // if(selected){
    //     dc.SetPen(wxPen(*wxBLACK, 1));
    //     const int hs = 6;
    //     wxPoint sp(pos.m_x * canvas->scale + canvas->offset.x, pos.m_y * canvas->scale + canvas->offset.y);
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


void Node::OpenPropertyDialog() {
    wxDialog dlg(canvas, wxID_ANY, "Node Properties");
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxTextCtrl *labelctrl = new wxTextCtrl(&dlg, wxID_ANY, wxString(label), wxDefaultPosition, wxDefaultSize);
    wxSpinCtrl* portCount = new wxSpinCtrl(&dlg, wxID_ANY);
    portCount->SetRange(0, 10);
    portCount->SetValue(ports.size());

    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "Label:"), 0, wxALL, 5);
    sizer->Add(labelctrl, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "Port Count:"), 0, wxALL, 5);
    sizer->Add(portCount, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxButton(&dlg, wxID_OK), 0, wxALIGN_CENTER | wxALL, 10);

    dlg.SetSizerAndFit(sizer);

    if (dlg.ShowModal() == wxID_OK) {
        label = labelctrl->GetValue().ToStdString();
        SetPortCount(portCount->GetValue());
        canvas->RefreshTree();
        canvas->Refresh();
    }
}

std::string Node::Serialize() const {
    // TODO
    return nullptr;
}

Node* Node::Deserialize(const std::string& line, MainCanvas* p) {
    // TODO
    return nullptr;
}
