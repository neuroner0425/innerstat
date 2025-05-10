#include <wx/dcbuffer.h>

#include <sstream>
#include <fstream>
#include <unordered_map>

#include "Canvas.h"
#include "Shape.h"

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

void MyCanvas::AddNewArea(const std::string& areaType) {
    shapes.push_back(new Area(100, 100 + shapes.size() * 30, 150, 80, areaType));
    selectedIndex = shapes.size() - 1;
    RefreshList();
    shapeList->SetSelection(selectedIndex);
    Refresh();
}

void MyCanvas::AddNewNode(const std::string& nodeType) {
    shapes.push_back(new Node(300, 100 + shapes.size() * 30, 100, 50, nodeType));
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

    for (const auto& c : connections)
        c.Draw(dc, scale, offset);

    for (size_t i = 0; i < shapes.size(); ++i)
        shapes[i]->Draw(dc, scale, offset, i == selectedIndex);
}

void MyCanvas::OnMouseWheel(wxMouseEvent& evt) {
    // 휠 방향에 따라 배율 조정값 결정
    double factor = (evt.GetWheelRotation() > 0) ? 1.1 : 1.0 / 1.1;

    wxPoint mouse = evt.GetPosition();

    // 1. 확대/축소 전 마우스 위치의 "논리 좌표" 계산
    wxPoint2DDouble before((mouse.x - offset.m_x) / scale,
                           (mouse.y - offset.m_y) / scale);

    // 2. 배율 적용 (최소/최대 배율 제한 포함)
    scale *= factor;
    if (scale < 0.1) scale = 0.1;
    if (scale > 10.0) scale = 10.0;

    // 3. 확대/축소 후의 "논리 좌표" 계산
    wxPoint2DDouble after((mouse.x - offset.m_x) / scale,
                          (mouse.y - offset.m_y) / scale);

    // 4. 줌인/아웃 시 마우스 기준으로 캔버스 위치를 보정 (줌 중심 유지)
    offset.m_x += (after.m_x - before.m_x) * scale;
    offset.m_y += (after.m_y - before.m_y) * scale;

    // 5. 다시 그리기
    Refresh();
}

void MyCanvas::OnLeftDown(wxMouseEvent& evt) {
    wxPoint mouse = evt.GetPosition();

    // 1. 클릭된 포트가 있는지 검사
    const Shape* clickedShape = nullptr;
    const Port* clickedPort = HitTestPort(mouse, &clickedShape);

    if (clickedPort) {
        // 2-1. 이미 연결 대기 중인 포트가 존재한다면, 연결 생성
        if (pendingPort && pendingPort != clickedPort) {
            connections.emplace_back(pendingPort, clickedPort, pendingShape, clickedShape);
            pendingPort = nullptr;
            pendingShape = nullptr;
        }
        // 2-2. 첫 포트를 선택한 경우 → 연결 대기
        else {
            pendingPort = clickedPort;
            pendingShape = clickedShape;
        }
        Refresh();
        return;
    }

    // 마우스 좌표 저장 및 초기화
    lastMouse = mouse;
    dragging = resizing = panning = false;
    activeHandle = HandleType::None;

    // 3. 도형 핸들 클릭 검사 (크기 조절 여부 판단)
    for (int i = shapes.size() - 1; i >= 0; --i) {
        activeHandle = shapes[i]->HitTestHandle(mouse, scale, offset);
        if (activeHandle != HandleType::None) {
            selectedIndex = i;
            resizing = true;
            shapeList->SetSelection(i);
            CaptureMouse();  // 마우스 캡처로 정확한 드래그 감지
            Refresh();
            return;
        }

        // 4. 도형 내부 클릭 시 → 도형 이동 시작
        if (shapes[i]->Contains(mouse, scale, offset)) {
            selectedIndex = i;
            dragging = true;
            shapeList->SetSelection(i);
            CaptureMouse();
            Refresh();
            return;
        }
    }

    // 5. 아무 도형도 클릭하지 않음 → 패닝 모드
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
    // 드래그 상태가 아니거나 왼쪽 버튼이 눌려있지 않다면 무시
    if (!evt.Dragging() || !evt.LeftIsDown()) return;

    wxPoint now = evt.GetPosition();
    wxPoint delta = now - lastMouse;
    lastMouse = now;

    // 1. 도형 이동 처리
    if (dragging && selectedIndex >= 0) {
        shapes[selectedIndex]->pos.m_x += delta.x / scale;
        shapes[selectedIndex]->pos.m_y += delta.y / scale;
    }

    // 2. 도형 크기 조절 처리
    else if (resizing && selectedIndex >= 0) {
        Shape* s = shapes[selectedIndex];
        double dx = delta.x / scale, dy = delta.y / scale;

        switch (activeHandle) {
            case HandleType::Right:        s->width += dx; break;
            case HandleType::Bottom:       s->height += dy; break;
            case HandleType::BottomRight:  s->width += dx; s->height += dy; break;
            case HandleType::Left:         s->pos.m_x += dx; s->width -= dx; break;
            case HandleType::Top:          s->pos.m_y += dy; s->height -= dy; break;
            case HandleType::TopLeft:      s->pos.m_x += dx; s->width -= dx; s->pos.m_y += dy; s->height -= dy; break;
            case HandleType::TopRight:     s->pos.m_y += dy; s->height -= dy; s->width += dx; break;
            case HandleType::BottomLeft:   s->pos.m_x += dx; s->width -= dx; s->height += dy; break;
            default: break;
        }

        // 최소 크기 보장
        if (s->width < 30) s->width = 30;
        if (s->height < 20) s->height = 20;
    }

    // 3. 캔버스 전체 이동 (패닝)
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