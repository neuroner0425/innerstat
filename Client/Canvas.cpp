#include "Canvas.h"
#include "Shape.h"
#include <wx/dcbuffer.h>
#include <sstream>
#include <fstream>
#include <unordered_map>

wxBEGIN_EVENT_TABLE(MyCanvas, wxPanel)
EVT_PAINT(MyCanvas::OnPaint)
EVT_MOUSEWHEEL(MyCanvas::OnMouseWheel)
EVT_LEFT_DOWN(MyCanvas::OnLeftDown)
EVT_LEFT_UP(MyCanvas::OnLeftUp)
EVT_MOTION(MyCanvas::OnMotion)
wxEND_EVENT_TABLE()

MyCanvas::MyCanvas(wxWindow* parent, wxListBox* list)
    : wxPanel(parent), shapeList(list) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_LEFT_DCLICK, &MyCanvas::OnLeftDClick, this);
}

MyCanvas::~MyCanvas() {
    for (auto* shape : shapes)
        delete shape;
}

/**
 * 에어리어 추가
 */
void MyCanvas::AddNewArea() {
    shapes.push_back(new Area(100, 100 + shapes.size() * 30, 150, 80, "OS"));
    selectedIndex = shapes.size() - 1;
    RefreshList();
    shapeList->SetSelection(selectedIndex);
    Refresh();
}

void MyCanvas::AddNewNode() {
    shapes.push_back(new Node(300, 100 + shapes.size() * 30, 100, 50, "PID 1234"));
    selectedIndex = shapes.size() - 1;
    RefreshList();
    shapeList->SetSelection(selectedIndex);
    Refresh();
}

void MyCanvas::SelectShape(int index) {
    if (index >= 0 && index < static_cast<int>(shapes.size())) {
        selectedIndex = index;
        Refresh();
    }
}

void MyCanvas::SaveToFile(const std::string& path) {
    std::ofstream out(path);
    for (const auto* shape : shapes) {
        out << shape->Serialize() << "\n";
    }
    for (const auto& conn : connections) {
        out << "Connection " << conn.from->id << " " << conn.to->id << "\n";
    }
}

void MyCanvas::LoadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return;

    shapes.clear();
    connections.clear();

    std::string line;
    std::unordered_map<std::string, const Port*> portMap;

    while (std::getline(in, line)) {
        if (line.rfind("Area", 0) == 0) {
            auto* area = Area::Deserialize(line);
            for (const auto& port : area->GetPorts())
                portMap[port.id] = &port;
            shapes.push_back(area);
        } else if (line.rfind("Node", 0) == 0) {
            auto* node = Node::Deserialize(line);
            for (const auto& port : node->GetPorts())
                portMap[port.id] = &port;
            shapes.push_back(node);
        } else if (line.rfind("Connection", 0) == 0) {
            std::istringstream iss(line);
            std::string tag, fromID, toID;
            iss >> tag >> fromID >> toID;
            if (portMap.find(fromID) != portMap.end() && portMap.find(toID) != portMap.end()) {
                connections.emplace_back(portMap[fromID], portMap[toID]);
            }
        }
    }
    Refresh();
}


void MyCanvas::RefreshList() {
    shapeList->Clear();
    for (size_t i = 0; i < shapes.size(); ++i) {
        if (dynamic_cast<Area*>(shapes[i]))
            shapeList->Append(wxString::Format("Area %zu", i));
        else if (dynamic_cast<Node*>(shapes[i]))
            shapeList->Append(wxString::Format("Node %zu", i));
        else
            shapeList->Append(wxString::Format("Shape %zu", i));
    }
}

void MyCanvas::OnPaint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    // 연결선 먼저 그림 (아래 그려도 OK)
    for (const auto& c : connections)
        c.Draw(dc, scale, offset);

    // 도형 그리기
    for (size_t i = 0; i < shapes.size(); ++i)
        shapes[i]->Draw(dc, scale, offset, i == selectedIndex);
}

void MyCanvas::OnMouseWheel(wxMouseEvent& evt) {
    double factor = (evt.GetWheelRotation() > 0) ? 1.1 : 1.0 / 1.1;
    wxPoint mouse = evt.GetPosition();
    wxPoint2DDouble before((mouse.x - offset.m_x) / scale, (mouse.y - offset.m_y) / scale);

    scale *= factor;
    if (scale < 0.1) scale = 0.1;
    if (scale > 10.0) scale = 10.0;

    wxPoint2DDouble after((mouse.x - offset.m_x) / scale, (mouse.y - offset.m_y) / scale);
    offset.m_x += (after.m_x - before.m_x) * scale;
    offset.m_y += (after.m_y - before.m_y) * scale;

    Refresh();
}

void MyCanvas::OnLeftDown(wxMouseEvent& evt) {
    wxPoint mouse = evt.GetPosition();
    const Shape* clickedShape = nullptr;
    const Port* clickedPort = HitTestPort(mouse, &clickedShape);
    
    if (clickedPort) {
        if (pendingPort && pendingPort != clickedPort) {
            // 연결 중복 확인 생략 가능
            connections.emplace_back(pendingPort, clickedPort, pendingShape, clickedShape);
            pendingPort = nullptr;
            pendingShape = nullptr;
        } else {
            pendingPort = clickedPort;
            pendingShape = clickedShape;
        }
        Refresh();
        return;
    }

    if (pendingPort && pendingPort != clickedPort) {
        // 중복 연결 확인
        bool exists = false;
        for (const auto& conn : connections) {
            if ((conn.from == pendingPort && conn.to == clickedPort) ||
                (conn.from == clickedPort && conn.to == pendingPort)) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            connections.emplace_back(pendingPort, clickedPort, pendingShape, clickedShape);
        }
        pendingPort = nullptr;
        Refresh();
        return;
    }

    lastMouse = evt.GetPosition();
    dragging = resizing = panning = false;
    activeHandle = HandleType::None;

    for (int i = shapes.size() - 1; i >= 0; --i) {
        activeHandle = shapes[i]->HitTestHandle(lastMouse, scale, offset);
        if (activeHandle != HandleType::None) {
            selectedIndex = i;
            resizing = true;
            shapeList->SetSelection(i);
            CaptureMouse();
            Refresh(); return;
        }
        if (shapes[i]->Contains(lastMouse, scale, offset)) {
            selectedIndex = i;
            dragging = true;
            shapeList->SetSelection(i);
            CaptureMouse();
            Refresh(); return;
        }
    }

    selectedIndex = -1;
    shapeList->DeselectAll();
    panning = true;
    CaptureMouse();
    Refresh();
}

void MyCanvas::OnLeftUp(wxMouseEvent&) {
    if (dragging || resizing || panning) {
        dragging = resizing = panning = false;
        ReleaseMouse();
        Refresh();
    }
}

void MyCanvas::OnMotion(wxMouseEvent& evt) {
    if (!evt.Dragging() || !evt.LeftIsDown()) return;

    wxPoint now = evt.GetPosition();
    wxPoint delta = now - lastMouse;
    lastMouse = now;

    if (dragging && selectedIndex >= 0) {
        shapes[selectedIndex]->pos.m_x += delta.x / scale;
        shapes[selectedIndex]->pos.m_y += delta.y / scale;
    }
    else if (resizing && selectedIndex >= 0) {
        Shape* s = shapes[selectedIndex];
        double dx = delta.x / scale, dy = delta.y / scale;

        switch (activeHandle) {
            case HandleType::Right: s->width += dx; break;
            case HandleType::Bottom: s->height += dy; break;
            case HandleType::BottomRight: s->width += dx; s->height += dy; break;
            case HandleType::Left: s->pos.m_x += dx; s->width -= dx; break;
            case HandleType::Top: s->pos.m_y += dy; s->height -= dy; break;
            case HandleType::TopLeft: s->pos.m_x += dx; s->width -= dx; s->pos.m_y += dy; s->height -= dy; break;
            case HandleType::TopRight: s->pos.m_y += dy; s->height -= dy; s->width += dx; break;
            case HandleType::BottomLeft: s->pos.m_x += dx; s->width -= dx; s->height += dy; break;
            default: break;
        }

        if (s->width < 30) s->width = 30;
        if (s->height < 20) s->height = 20;
    }
    else if (panning) {
        offset.m_x += delta.x;
        offset.m_y += delta.y;
    }

    Refresh();
}

const Port* MyCanvas::HitTestPort(const wxPoint& pos, const Shape** outShape) const {
    for (const auto* shape : shapes) {
        const auto& ports = shape->GetPorts();
        for (const auto& port : ports) {
            wxPoint screenPos = port.GetScreenPosition(shape->pos, shape->width, shape->height, scale, offset);
            wxRect hitbox(screenPos.x - 6, screenPos.y - 6, 12, 12);
            if (hitbox.Contains(pos)) {
                if (outShape) *outShape = shape;
                return &port;
            }
        }
    }
    return nullptr;
}

void MyCanvas::OnLeftDClick(wxMouseEvent& evt) {
    wxPoint pos = evt.GetPosition();
    for (auto* shape : shapes) {
        if (shape->Contains(pos, scale, offset)) {
            shape->OpenPropertyDialog(this);
            Refresh();
            return;
        }
    }
}