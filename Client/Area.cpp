#include "Area.h"
#include <wx/textdlg.h>
#include <wx/spinctrl.h>
#include <sstream>

Area::Area(double x, double y, double w, double h, const std::string& t)
    : Shape(x, y, w, h), type(t) {
    SetPortCount(2);
}

void Area::SetPortCount(int count) {
    ports.clear();
    for (int i = 0; i < count; ++i) {
        double ratio = (i + 1.0) / (count + 1.0); // 0.25, 0.5, 0.75 ...
        ports.emplace_back("p" + std::to_string(i), wxPoint2DDouble(ratio, 0.0));
    }
}

void Area::OpenPropertyDialog(wxWindow *parent) {
    wxDialog dlg(parent, wxID_ANY, "Area Properties");
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    wxTextCtrl *typeCtrl = new wxTextCtrl(&dlg, wxID_ANY, type);
    wxSpinCtrl *portCount = new wxSpinCtrl(&dlg, wxID_ANY);
    portCount->SetRange(0, 10);
    portCount->SetValue(ports.size());

    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "Type:"), 0, wxALL, 5);
    sizer->Add(typeCtrl, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "Port Count:"), 0, wxALL, 5);
    sizer->Add(portCount, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxButton(&dlg, wxID_OK), 0, wxALIGN_CENTER | wxALL, 10);

    dlg.SetSizerAndFit(sizer);

    if (dlg.ShowModal() == wxID_OK) {
        type = typeCtrl->GetValue().ToStdString();
        SetPortCount(portCount->GetValue());
    }
}

void Area::Draw(wxDC& dc, double scale, const wxPoint2DDouble& offset, bool selected) const {
    wxPoint screenPos(pos.m_x * scale + offset.m_x, pos.m_y * scale + offset.m_y);
    int w = width * scale;
    int h = height * scale;

    dc.SetBrush(selected ? wxBrush(*wxBLUE) : wxBrush(wxColour(180, 180, 255)));
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawRoundedRectangle(screenPos.x, screenPos.y, w, h, 10);
    dc.DrawText("Area: " + type, screenPos + wxPoint(5, 5));

    for (const auto& port : ports) {
        wxPoint p = port.GetScreenPosition(pos, width, height, scale, offset);
        port.Draw(dc, p);
        dc.DrawText(port.id, p + wxPoint(6, -6));  // 포트 이름 표시
    }
}

std::string Area::Serialize() const {
    std::ostringstream oss;
    oss << "Area " << type << " " << pos.m_x << " " << pos.m_y << " " << width << " " << height << " " << ports.size();
    return oss.str();
}

Area* Area::Deserialize(const std::string& line) {
    std::istringstream iss(line);
    std::string tag, type;
    double x, y, w, h;
    int portCount;

    iss >> tag >> type >> x >> y >> w >> h >> portCount;
    auto* area = new Area(x, y, w, h, type);
    area->SetPortCount(portCount);
    return area;
}
