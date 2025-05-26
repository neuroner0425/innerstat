#include "Node.h"
#include "Canvas.h"
#include "Port.h"
#include <wx/textdlg.h>
#include <wx/spinctrl.h>
#include <sstream>

Node::Node(double x, double y, double w, double h, 
    MyCanvas* c, Shape* p, const std::string& l)
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
    wxPoint screenPos(pos.m_x * canvas->scale + canvas->offset.m_x, pos.m_y * canvas->scale + canvas->offset.m_y);
    int w = width * canvas->scale;
    int h = height * canvas->scale;

    wxColour fill = overloaded ? wxColour(255, 100, 100)
                  : (active ? wxColour(180, 255, 180) : wxColour(200, 200, 200));

    dc.SetBrush(selected ? wxBrush(*wxYELLOW) : wxBrush(fill));
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawRoundedRectangle(screenPos.x, screenPos.y, w, h, 6);
    dc.DrawText("PID: " + label, screenPos + wxPoint(5, 5));

    for (const auto& port : ports) {
        wxPoint p = port.GetScreenPosition(pos, width, height, canvas->scale, canvas->offset);
        port.Draw(dc, p);
        dc.DrawText(port.id, p + wxPoint(6, -6));
    }
}

void Node::OpenPropertyDialog(MyCanvas* canvas) {
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

Node* Node::Deserialize(const std::string& line, MyCanvas* p) {
    // TODO
    return nullptr;
}
