#include "Node.h"
#include <wx/textdlg.h>
#include <wx/spinctrl.h>
#include <sstream>
#include <fstream>
#include <unordered_map>

Node::Node(double x, double y, double w, double h, const std::string &pid)
    : Shape(x, y, w, h), pidIdentifier(pid), active(true), overloaded(false)
{
    SetPortCount(2);
}

void Node::SetPortCount(int count)
{
    ports.clear();
    for (int i = 0; i < count; ++i)
    {
        double ratio = (i + 1.0) / (count + 1.0); // 0.25, 0.5, 0.75 ...
        ports.emplace_back("p" + std::to_string(i), wxPoint2DDouble(ratio, 0.0));
    }
}


void Node::Draw(wxDC &dc, double scale, const wxPoint2DDouble &offset, bool selected) const
{
    wxPoint screenPos(pos.m_x * scale + offset.m_x, pos.m_y * scale + offset.m_y);
    int w = width * scale;
    int h = height * scale;

    wxColour fill = overloaded ? wxColour(255, 100, 100) : (active ? wxColour(180, 255, 180) : wxColour(200, 200, 200));
    dc.SetBrush(selected ? wxBrush(*wxYELLOW) : wxBrush(fill));
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawRoundedRectangle(screenPos.x, screenPos.y, w, h, 6);

    dc.DrawText("PID: " + pidIdentifier, screenPos + wxPoint(5, 5));
    for (const auto& port : ports) {
        wxPoint p = port.GetScreenPosition(pos, width, height, scale, offset);
        port.Draw(dc, p);
        dc.DrawText(port.id, p + wxPoint(6, -6));
    }
}

void Node::OpenPropertyDialog(wxWindow* parent) {
    wxDialog dlg(parent, wxID_ANY, "Node Properties");
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxTextCtrl* pidCtrl = new wxTextCtrl(&dlg, wxID_ANY, pidIdentifier);
    wxSpinCtrl* portCount = new wxSpinCtrl(&dlg, wxID_ANY);
    portCount->SetRange(0, 10);
    portCount->SetValue(ports.size());

    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "PID Identifier:"), 0, wxALL, 5);
    sizer->Add(pidCtrl, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "Port Count:"), 0, wxALL, 5);
    sizer->Add(portCount, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxButton(&dlg, wxID_OK), 0, wxALIGN_CENTER | wxALL, 10);

    dlg.SetSizerAndFit(sizer);
    if (dlg.ShowModal() == wxID_OK) {
        pidIdentifier = pidCtrl->GetValue().ToStdString();

        ports.clear();
        int count = portCount->GetValue();
        for (int i = 0; i < count; ++i) {
            double ratio = (i + 1.0) / (count + 1.0);
            ports.emplace_back("p" + std::to_string(i), wxPoint2DDouble(ratio, 0.0));
        }
    }
}

std::string Node::Serialize() const {
    std::ostringstream oss;
    oss << "Node " << pidIdentifier << " " << pos.m_x << " " << pos.m_y << " " << width << " " << height << " " << ports.size();
    return oss.str();
}

Node* Node::Deserialize(const std::string& line) {
    std::istringstream iss(line);
    std::string tag, pid;
    double x, y, w, h;
    int portCount;
    iss >> tag >> pid >> x >> y >> w >> h >> portCount;
    auto* node = new Node(x, y, w, h, pid);
    node->SetPortCount(portCount);
    return node;
}
