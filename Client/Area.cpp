#include "Area.h"
#include "Node.h"
#include "Canvas.h"
#include "Port.h"
#include <wx/textdlg.h>
#include <wx/spinctrl.h>
#include <sstream>

Area::Area(double x, double y, double w, double h, const std::string& t, MyCanvas* p)
    : Shape(x, y, w, h, p), type(t) {
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
        double ratio = (i + 1.0) / (count + 1.0); // 0.25, 0.5, 0.75 ...
        ports.emplace_back("p" + std::to_string(i), wxPoint2DDouble(ratio, 0.0));
    }
}

void Area::OpenPropertyDialog(MyCanvas *parent) {
    wxDialog dlg(parent, wxID_ANY, "Area Properties");
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    wxTextCtrl *typeCtrl = new wxTextCtrl(&dlg, wxID_ANY, type);
    wxSpinCtrl *portCount = new wxSpinCtrl(&dlg, wxID_ANY);
    wxButton *addShapeBtn = new wxButton(&dlg, wxID_ANY, "Add Shape");

    portCount->SetRange(0, 10);
    portCount->SetValue(ports.size());

    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "Type:"), 0, wxALL, 5);
    sizer->Add(typeCtrl, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxStaticText(&dlg, wxID_ANY, "Port Count:"), 0, wxALL, 5);
    sizer->Add(portCount, 0, wxEXPAND | wxALL, 5);
    sizer->Add(addShapeBtn, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxButton(&dlg, wxID_OK), 0, wxALIGN_CENTER | wxALL, 10);

    dlg.SetSizerAndFit(sizer);

    addShapeBtn->Bind(wxEVT_BUTTON,[=](wxCommandEvent &evt){
        this->OpenAddShapeDialog(parent);
    });

    if (dlg.ShowModal() == wxID_OK) {
        type = typeCtrl->GetValue().ToStdString();
        SetPortCount(portCount->GetValue());
    }
}

void Area::OpenAddShapeDialog(MyCanvas* parent){
    wxArrayString choices;
    choices.Add("Add Sub-Area");
    choices.Add("Add Node");

    wxSingleChoiceDialog choiceDlg(parent, "무엇을 추가하시겠습니까?", "Add to Area", choices);
    if (choiceDlg.ShowModal() == wxID_OK) {
        int sel = choiceDlg.GetSelection();

        if (sel == 0) {
            // 하위 Area 추가
            wxTextEntryDialog nameDlg(parent, "Sub-Area-type", "Sub-Area", "OS");
            if (nameDlg.ShowModal() == wxID_OK) {
                std::string subType = nameDlg.GetValue().ToStdString();
                double offsetX = this->pos.m_x + 30 + this->GetSubAreas().size() * 20;
                double offsetY = this->pos.m_y + 40;
                Area* newArea = new Area(offsetX, offsetY, 150, 80, subType, parent);
                this->AddSubArea(newArea);
            }
        } else if (sel == 1) {
            // Node 추가
            wxTextEntryDialog pidDlg(parent, "NodePID", "Node", "PID0000");
            if (pidDlg.ShowModal() == wxID_OK) {
                std::string pid = pidDlg.GetValue().ToStdString();
                double offsetX = this->pos.m_x + 30 + this->GetNodes().size() * 20;
                double offsetY = this->pos.m_y + 100;
                Node* newNode = new Node(offsetX, offsetY, 100, 50, pid, parent);
                this->AddNode(newNode);
            }
        }

        parent->RefreshTree();
        parent->Refresh();
        return;
    }
}


void Area::Draw(wxDC& dc) const {
    wxPoint screenPos(pos.m_x * parent->scale + parent->offset.m_x, pos.m_y * parent->scale + parent->offset.m_y);
    int w = width * parent->scale;
    int h = height * parent->scale;

    dc.SetBrush(selected ? wxBrush(*wxBLUE) : wxBrush(wxColour(180, 180, 255)));
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawRoundedRectangle(screenPos.x, screenPos.y, w, h, 10);
    dc.DrawText("Area: " + type, screenPos + wxPoint(5, 5));

    for (const Port& port : ports) {
        wxPoint p = port.GetScreenPosition(pos, width, height, parent->scale, parent->offset);
        port.Draw(dc, p);
        dc.DrawText(port.id, p + wxPoint(6, -6));  // 포트 이름 표시
    }

    // 자식 도형들도 마지막에 그리기
    for (const Shape* shape : subAreas){
        shape->Draw(dc);
    }
    for (const Shape* shape : nodes){
        shape->Draw(dc);
    }
}

std::string Area::Serialize() const {
    std::ostringstream oss;
    oss << "Area " << type << " " << pos.m_x << " " << pos.m_y << " " << width << " " << height << " " << ports.size();
    return oss.str();
}

Area* Area::Deserialize(const std::string& line, MyCanvas* parent) {
    std::istringstream iss(line);
    std::string tag, type;
    double x, y, w, h;
    int portCount;

    iss >> tag >> type >> x >> y >> w >> h >> portCount;
    Area* area = new Area(x, y, w, h, type, parent);
    area->SetPortCount(portCount);
    return area;
}


void Area::AddSubArea(Area* area) {
    subAreas.push_back(area);
}

void Area::AddNode(Node* node) {
    nodes.push_back(node);
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