#include <wx/dcbuffer.h>
#include <stdio.h>

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

MyCanvas::MyCanvas(wxWindow* parent, wxTreeCtrl* t)
    : wxPanel(parent), shapeTree(t) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_LEFT_DCLICK, &MyCanvas::OnLeftDClick, this);
}

MyCanvas::~MyCanvas() {
    for (Area* area : areas)
        delete area;
    
    areas.clear();
}

void MyCanvas::AddNewArea(const std::string& areaType) {
    areas.push_back(new Area(100, 100 + areas.size() * 30, 150, 80, areaType, this));
    selectedShape = areas.back();
    RefreshTree();
    // TODO
    // shapeTree->SetSelection(selectedShape);
    Refresh();
}

void MyCanvas::SaveToFile(const std::string& path) {
    std::ofstream out(path);
    for (const Area* area : areas) {
        out << area->Serialize() << "\n";
    }
    for (const Connection& conn : connections) {
        out << "Connection " << conn.from->id << " " << conn.to->id << "\n";
    }
}

void MyCanvas::LoadFromFile(const std::string& path) {
    // TODO
}

void MyCanvas::RefreshTree() {
    shapeTree->DeleteAllItems();
    shapeMap.clear();

    wxTreeItemId root = shapeTree->AddRoot("Systems");

    for (size_t i = 0; i < areas.size(); ++i) {
        if (auto* area = dynamic_cast<Area*>(areas[i])) {
            AppendAreaToTree(root, area);
        }
    }

    shapeTree->ExpandAll();
}

void MyCanvas::OnPaint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    
    for (size_t i = 0; i < areas.size(); ++i)
        areas[i]->Draw(dc);

    for (const Connection& c : connections)
        c.Draw(dc, scale, offset);
    
    dc.SetBrush(wxBrush(*wxRED
    ));
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawCircle(wxPoint(offset.m_x, offset.m_y), 10*scale);
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
    const Port* clickedPort = nullptr;

    for(Area* area : areas) if((clickedPort = area->HitTestPort(mouse, &clickedShape))) break;

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
    for(Area* area : areas) if(area->HitTestShape(mouse)) return;

    // 5. 아무 도형도 클릭하지 않음 → 패닝 모드
    UnSelectShape();
    shapeTree->UnselectAll();
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
    if (dragging && selectedShape != nullptr) {
        selectedShape->pos.m_x += delta.x / scale;
        selectedShape->pos.m_y += delta.y / scale;
    }

    // 2. 도형 크기 조절 처리
    else if (resizing && selectedShape != nullptr) {
        Shape* s = selectedShape;
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

void MyCanvas::OnLeftDClick(wxMouseEvent& evt) {
    wxPoint pos = evt.GetPosition();
    for (std::vector<Area*>::const_reverse_iterator it = areas.rbegin(); it != areas.rend(); ++it) {
        if((*it)->OpenProperty(pos)) return;
    }
}

void MyCanvas::ResizingShape(Shape* shape, HandleType& handleType){
    this->activeHandle = handleType;
    SelectShape(shape);
    resizing = true;
    // TODO
    // shapeTree->SetSelection(i);
    CaptureMouse();  // 마우스 캡처로 정확한 드래그 감지
    Refresh();
}

void MyCanvas::DraggingShape(Shape* shape){
    SelectShape(shape);
    dragging = true;
    // TODO
    // shapeTree->SetSelection(i);
    CaptureMouse();
    Refresh();
}

void MyCanvas::AppendAreaToTree(wxTreeItemId parentId, Area* area) {
    wxString label = wxString::Format("Area [%s]", area->getType());
    wxTreeItemId areaId = shapeTree->AppendItem(parentId, label);
    shapeMap[areaId] = area;

    for (auto* sub : area->GetSubAreas()) {
        AppendAreaToTree(areaId, sub);
    }
    for (auto* node : area->GetNodes()) {
        wxString nlabel = wxString::Format("Node [%s]", node->pidIdentifier);
        wxTreeItemId nid = shapeTree->AppendItem(areaId, nlabel);
        shapeMap[nid] = node;
    }
}

void MyCanvas::OnTreeSelectionChanged(wxTreeItemId itemId) {
    auto it = shapeMap.find(itemId);
    if (it != shapeMap.end()) {
        Shape* s = it->second;
        for (size_t i = 0; i < areas.size(); ++i) {
            if (areas[i] == s) {
                SelectShape(areas[i]);
                Refresh();
                return;
            }
        }
    }
}
