#include <wx/dcbuffer.h>
#include <stdio.h>

#include <sstream>
#include <fstream>
#include <unordered_map>

#include "Canvas.h"
#include "Shape.h"
#include "Area.h"

MyCanvas::MyCanvas(wxWindow* parent, wxTreeCtrl* t)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS), shapeTree(t) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &MyCanvas::OnPaint, this);
    Bind(wxEVT_MOUSEWHEEL, &MyCanvas::OnMouseWheel, this);
    Bind(wxEVT_LEFT_DOWN, &MyCanvas::OnLeftDown, this);
    Bind(wxEVT_LEFT_UP, &MyCanvas::OnLeftUp, this);
    Bind(wxEVT_MIDDLE_DOWN, &MyCanvas::OnMiddleDown, this);
    Bind(wxEVT_MIDDLE_UP, &MyCanvas::OnMiddleUp, this);
    Bind(wxEVT_KEY_DOWN, &MyCanvas::OnKeyDown, this);
    Bind(wxEVT_KEY_UP, &MyCanvas::OnKeyUp, this);
    Bind(wxEVT_MOTION, &MyCanvas::OnMotion, this);
    Bind(wxEVT_LEFT_DCLICK, &MyCanvas::OnLeftDClick, this);
    Bind(wxEVT_IDLE, &MyCanvas::OnFirstIdle, this);
    this->SetFocus();
}

MyCanvas::~MyCanvas() {
    for (std::vector<Area*>::const_reverse_iterator it = areas.rbegin(); it != areas.rend(); ++it)
        delete (*it);
    areas.clear();
}

void MyCanvas::AddNewArea(const std::string& label, const AreaType areaType) {
    areas.push_back(new Area(100, 100 + areas.size() * 30, 150, 150, this, nullptr, "New Area", areaType));
    selectedShape = areas.back();
    RefreshTree();
    // TODO
    // shapeTree->SetSelection(selectedShape);
    UpdateAllShapesList();
    Refresh();
}

void MyCanvas::SaveToFile(const std::string& path) {
    std::ofstream out(path);
    for (std::vector<Area*>::const_reverse_iterator it = areas.rbegin(); it != areas.rend(); ++it) {
        out << (*it)->Serialize() << "\n";
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

    for (Shape* s : allShapes)
        s->Draw(dc);

    for (const Connection& c : connections)
        c.Draw(dc, scale, offset);

    if (isDrawingConnection && pendingPort) {
        wxPoint start = pendingPort->GetScreenPosition(
            pendingShape->pos, pendingShape->width, pendingShape->height, scale, offset);
        wxPoint end = pendingMousePos;
        dc.SetPen(wxPen(*wxBLACK, (int)(2 * scale), wxPENSTYLE_DOT));
        dc.DrawLine(start, end);
    }
    
    dc.SetBrush(wxBrush(*wxRED));
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawCircle(wxPoint((int)offset.m_x, (int)offset.m_y), (int)(scale * 10));
    dc.DrawText(wxString::Format("Scale: %.2f", scale), wxPoint(0, 0));
    this->SetFocus();
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

    if (middleMouseDown) return;
    
    if (spacePressed) {
        StartPanning();
        return;
    }

    // 1. 클릭된 포트가 있는지 검사
    const Shape* clickedShape = nullptr;
    const Port* clickedPort = nullptr;

    for(std::vector<Area*>::const_reverse_iterator it = areas.rbegin(); it != areas.rend(); ++it) if((clickedPort = (*it)->HitTestPort(mouse, &clickedShape))) break;

    if (clickedPort) {
        pendingPort = clickedPort;
        pendingShape = clickedShape;
        isDrawingConnection = true;
        pendingMousePos = mouse;
        CaptureMouse();
        return;
    }

    // 마우스 좌표 저장 및 초기화
    lastMouse = mouse;
    dragging = resizing = false;
    activeHandle = HandleType::None;

    // 3. 도형 핸들 클릭 검사 (크기 조절 여부 판단)
    for(std::vector<Area*>::const_reverse_iterator it = areas.rbegin(); it != areas.rend(); ++it) if((*it)->HitTestShape(mouse)) return;

    // 5. 아무 도형도 클릭하지 않음 → 패닝 모드
    UnSelectShape();
    shapeTree->UnselectAll();

    // panning = true;
    // CaptureMouse();

    Refresh();
}

void MyCanvas::OnLeftUp(wxMouseEvent& evt) {
    if (isDrawingConnection) {
        wxPoint mouse = evt.GetPosition();
        const Shape* hoveredShape = nullptr;
        const Port* hoveredPort = nullptr;
        for(auto it = areas.rbegin(); it != areas.rend(); ++it)
            if((hoveredPort = (*it)->HitTestPort(mouse, &hoveredShape))) break;

        // 다른 포트 위에서 놓았을 때만 연결
        if (hoveredPort && hoveredPort != pendingPort) {
            connections.emplace_back(pendingPort, hoveredPort, pendingShape, hoveredShape);
        }
        isDrawingConnection = false;
        pendingPort = nullptr;
        pendingShape = nullptr;
        ReleaseMouse();
        Refresh();
        return;
    }

    if (dragging || resizing) {
        dragging = resizing = false;
        ReleaseMouse();
        Refresh();
    }
}

void MyCanvas::OnMotion(wxMouseEvent& evt) {
    wxPoint now = evt.GetPosition();
    wxPoint delta = now - lastMouse;
    lastMouse = now;

    
    if (isDrawingConnection) {
        pendingMousePos = evt.GetPosition();
        Refresh(); // Paint에서 임시선 그림
        return;
    }

    // 패닝 조건
    if ((spacePressed && evt.LeftIsDown() && evt.Dragging()) ||
        (middleMouseDown && evt.MiddleIsDown() && evt.Dragging())) {
        offset.m_x += delta.x;
        offset.m_y += delta.y;
        Refresh();
        return;
    }
    
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
    wxString label = wxString::Format("%s [%s]", area->label, area->getTypeStr());
    wxTreeItemId areaId = shapeTree->AppendItem(parentId, label);
    shapeMap[areaId] = area;

    for (auto* sub : area->GetSubAreas()) {
        AppendAreaToTree(areaId, sub);
    }
    for (auto* node : area->GetNodes()) {
        wxString nlabel = wxString::Format("%s [PS]", node->label);
        wxTreeItemId nid = shapeTree->AppendItem(areaId, nlabel);
        shapeMap[nid] = node;
    }
}

void MyCanvas::OnTreeSelectionChanged(wxTreeItemId itemId) {
    auto it = shapeMap.find(itemId);
    if (it != shapeMap.end()) {
        Shape* s = it->second;
        for (Shape* shape : allShapes) {
            if (shape == s) {
                SelectShape(shape);
                Refresh();
                return;
            }
        }
    }
}

void MyCanvas::OnTreeLeftDClick(wxTreeItemId itemId) {
    auto it = shapeMap.find(itemId);
    if (it != shapeMap.end()) {
        Shape* s = it->second;
        for (Shape* shape : allShapes) {
            if (shape == s) {
                shape->OpenPropertyDialog(this);
                return;
            }
        }
    }
}

void MyCanvas::OnFirstIdle(wxIdleEvent& evt) {
    wxSize size = GetClientSize();
    offset = wxPoint2DDouble(size.GetWidth() / 2, size.GetHeight() / 5);

    Unbind(wxEVT_IDLE, &MyCanvas::OnFirstIdle, this);  // 한 번만 실행되도록 해제
    Refresh();  // 화면 다시 그리기
}

void MyCanvas::UpdateAllShapesList() {
    allShapes.clear();

    std::function<void(Area*)> recurse = [&](Area* area) {
        allShapes.push_back(area);
        for (Area* sub : area->GetSubAreas())
            recurse(sub);
        for (Node* node : area->GetNodes())
            allShapes.push_back(node);
    };

    for (Area* a : areas) {
        recurse(a);
    }
}

void MyCanvas::OnKeyDown(wxKeyEvent& evt) {
    if (evt.GetKeyCode() == WXK_SPACE) {
        spacePressed = true;
        evt.StopPropagation();
        SetCursor(wxCursor(wxCURSOR_HAND));
        return;
    }
    evt.Skip();
}

void MyCanvas::OnKeyUp(wxKeyEvent& evt) {
    if (evt.GetKeyCode() == WXK_SPACE) {
        spacePressed = false;
        evt.StopPropagation();
        if(!middleMouseDown){
            StopPanning();
        }
        return;
    }
    evt.Skip();
}

void MyCanvas::OnMiddleDown(wxMouseEvent& evt) {
    lastMouse = evt.GetPosition();
    middleMouseDown = true;
    StartPanning();
}

void MyCanvas::OnMiddleUp(wxMouseEvent& evt) {
    middleMouseDown = false;
    StopPanning();
}

void MyCanvas::StartPanning(){
    if(!panning){
        panning = true;
        SetCursor(wxCursor(wxCURSOR_HAND));
        CaptureMouse();
    }
}

void MyCanvas::StopPanning(){
    panning = false;
    SetCursor(wxCursor(*wxSTANDARD_CURSOR));
    if (HasCapture()) ReleaseMouse();
}