#include <wx/dcbuffer.h>
#include <stdio.h>

#include <sstream>
#include <fstream>
#include <unordered_map>
#include <cmath>

#include "Canvas.h"
#include "Shape.h"
#include "Area.h"

MainCanvas::MainCanvas(wxWindow* parent, wxTreeCtrl* t)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS), shapeTree(t) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &MainCanvas::OnPaint, this);
    Bind(wxEVT_MOUSEWHEEL, &MainCanvas::OnMouseWheel, this);
    Bind(wxEVT_LEFT_DOWN, &MainCanvas::OnLeftDown, this);
    Bind(wxEVT_LEFT_UP, &MainCanvas::OnLeftUp, this);
    Bind(wxEVT_MIDDLE_DOWN, &MainCanvas::OnMiddleDown, this);
    Bind(wxEVT_MIDDLE_UP, &MainCanvas::OnMiddleUp, this);
    Bind(wxEVT_KEY_DOWN, &MainCanvas::OnKeyDown, this);
    Bind(wxEVT_KEY_UP, &MainCanvas::OnKeyUp, this);
    Bind(wxEVT_MOTION, &MainCanvas::OnMotion, this);
    Bind(wxEVT_LEFT_DCLICK, &MainCanvas::OnLeftDClick, this);
    Bind(wxEVT_IDLE, &MainCanvas::OnFirstIdle, this);
    this->SetFocus();
}

MainCanvas::~MainCanvas() {
    for (std::vector<Area*>::const_reverse_iterator it = areas.rbegin(); it != areas.rend(); ++it)
        delete (*it);
    areas.clear();
}

void MainCanvas::AddNewArea(const std::string& label, const AreaType areaType) {
    areas.push_back(new Area(120, 120 + areas.size() * 40, 120, 120, this, nullptr, label, areaType));
    // selectedShape = areas.back();
    RefreshTree();
    // shapeTree->SetSelection(selectedShape);
    UpdateAllShapesList();
    Refresh();
}

void MainCanvas::SaveToFile(const std::string& path) {
    // TODO
}

void MainCanvas::LoadFromFile(const std::string& path) {
    // TODO
}

void MainCanvas::RefreshTree() {
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

wxPoint MainCanvas::SnapToGrid(const wxPoint& pt) {
    int snappedX = (pt.x >= 0)
        ? ((pt.x + GRID_SIZE / 2) / GRID_SIZE) * GRID_SIZE
        : ((pt.x - GRID_SIZE / 2) / GRID_SIZE) * GRID_SIZE;

    int snappedY = (pt.y >= 0)
        ? ((pt.y + GRID_SIZE / 2) / GRID_SIZE) * GRID_SIZE
        : ((pt.y - GRID_SIZE / 2) / GRID_SIZE) * GRID_SIZE;

    return wxPoint(snappedX, snappedY);
}

void MainCanvas::OnPaint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    
    // 1. 그리드
    wxSize sz = GetClientSize();
    dc.SetPen(wxPen(wxColour(230,230,230)));
    double mid_x = std::fmod(offset.m_x, GRID_SIZE * scale);
    double mid_y = std::fmod(offset.m_y, GRID_SIZE * scale);
    dc.DrawLine(mid_x, 0, mid_x, sz.GetHeight());
    dc.DrawLine(0, mid_y, sz.GetWidth(), mid_y);
    for (double x = GRID_SIZE * scale; x < sz.GetWidth(); x += GRID_SIZE * scale){
        dc.DrawLine(mid_x + x, 0, mid_x + x, sz.GetHeight());
        dc.DrawLine(mid_x - x, 0, mid_x - x, sz.GetHeight());
    }
    for (double y = GRID_SIZE * scale; y < sz.GetHeight(); y += GRID_SIZE * scale){
        dc.DrawLine(0, mid_y + y, sz.GetWidth(), mid_y + y);
        dc.DrawLine(0, mid_y - y, sz.GetWidth(), mid_y - y);
    }
    
    if (dragging && showPreview && selectedShape) {
        wxRect rect(selectedShape->position);
        rect.SetPosition(previewPos);
        wxPoint screenPos(rect.x * scale + offset.m_x, rect.y * scale + offset.m_y);
        int w = (int)(rect.width * scale);
        int h = (int)(rect.height * scale);

        wxBrush brush(wxColour(150, 200, 255, 90));
        wxPen pen(wxColour(80,80,200,170), 2, wxPENSTYLE_DOT_DASH);
        dc.SetBrush(brush);
        dc.SetPen(pen);
        dc.DrawRoundedRectangle(screenPos.x, screenPos.y, w, h, (int)(10 * scale));
    }

    // 도형
    for (const Shape* s : allShapes)
        s->Draw(dc);

    // 연결선
    for (const Connection& c : connections)
        c.Draw(dc, scale, offset);

    // 연결선 미리보기
    if (isDrawingConnection && pendingPort) {
        wxRect pendingShapePosition(pendingShape->position);
        wxPoint start = pendingPort->GetScreenPosition(
            pendingShapePosition.GetPosition(), pendingShapePosition.width, pendingShapePosition.height, scale, offset);
        wxPoint end = pendingMousePos;
        dc.SetPen(wxPen(*wxBLACK, (int)(2 * scale), wxPENSTYLE_DOT));
        dc.DrawLine(start, end);
    }
    
    // 디버깅용
    if(isdebug){    
        dc.SetBrush(wxBrush(*wxRED));
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawCircle(wxPoint((int)offset.m_x, (int)offset.m_y), (int)(scale * 8));
        dc.DrawText(wxString::Format("Scale: %.2f", scale), wxPoint(0, 0));
        if(dragging){
            dc.DrawText(wxString::Format("start_position(%d, %d)", dragStartMousePosition.x, dragStartMousePosition.y), wxPoint(0, 20));
            dc.DrawText(wxString::Format("current_position(%d, %d)", lastMouse.x, lastMouse.y), wxPoint(0, 40));
            dc.DrawText(wxString::Format("start_offset(%d, %d)", dragOffset.x, dragOffset.y), wxPoint(0, 60));
            dc.DrawText(wxString::Format("now_offset(%d, %d)", previewPos.x, previewPos.y), wxPoint(0, 80));
        }
    }
}


void MainCanvas::OnMouseWheel(wxMouseEvent& evt) {
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

void MainCanvas::OnLeftDown(wxMouseEvent& evt) {
    wxPoint mouse = evt.GetPosition();
    lastMouse = mouse;

    if (middleMouseDown) return;
    
    if (spacePressed) {
        StartPanning();
        return;
    }

    // 1. 클릭된 포트가 있는지 검사
    const Shape* clickedShape = nullptr;
    const Port* clickedPort = nullptr;

    for(std::vector<Area*>::const_reverse_iterator it = areas.rbegin(); it != areas.rend(); ++it)
        if((clickedPort = (*it)->HitTestPort(mouse, &clickedShape))) break;

    if (clickedPort) {
        pendingPort = clickedPort;
        pendingShape = clickedShape;
        isDrawingConnection = true;
        pendingMousePos = mouse;
        CaptureMouse();
        return;
    }

    // 마우스 좌표 저장 및 초기화
    dragging = resizing = false;
    activeHandle = HandleType::None;

    // 3. 도형 핸들 클릭 검사 (크기 조절 여부 판단)
    for(std::vector<Area*>::const_reverse_iterator it = areas.rbegin(); it != areas.rend(); ++it)
        if((*it)->HitTestShape(mouse)) return;

    // 5. 아무 도형도 클릭하지 않음 → 패닝 모드
    UnSelectShape();
    shapeTree->UnselectAll();

    // panning = true;
    // CaptureMouse();

    Refresh();
}

void MainCanvas::OnLeftUp(wxMouseEvent& evt) {
    if (HasCapture()) ReleaseMouse();
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
        Refresh();
        return;
    }

    if (dragging) {
        selectedShape->SetPosition(previewPos);
        dragging = false;
        showPreview = false;
        Refresh();
        return;
    }

    if (resizing) {
        resizing = false;
        Refresh();
    }
}

void MainCanvas::OnMotion(wxMouseEvent& evt) {
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
        wxPoint shapePos = (now - dragStartMousePosition);
        shapePos.x /= scale;
        shapePos.y /= scale;
        shapePos += dragOffset;
        selectedShape->SetPosition(shapePos);
        // selectedShape->position.x += delta.x / scale;
        // selectedShape->position.y += delta.y / scale;

        previewPos = SnapToGrid(shapePos);
        showPreview = true;
    }

    // 2. 도형 크기 조절 처리
    else if (resizing && selectedShape != nullptr) {
        Shape* s = selectedShape;
        double dx = delta.x / scale, dy = delta.y / scale;

        switch (activeHandle) {
            case HandleType::Right:        s->position.width += dx; break;
            case HandleType::Bottom:       s->position.height += dy; break;
            case HandleType::BottomRight:  s->position.width += dx; s->position.height += dy; break;
            case HandleType::Left:         s->position.x += dx; s->position.width -= dx; break;
            case HandleType::Top:          s->position.y += dy; s->position.height -= dy; break;
            case HandleType::TopLeft:      s->position.x += dx; s->position.width -= dx; s->position.y += dy; s->position.height -= dy; break;
            case HandleType::TopRight:     s->position.y += dy; s->position.height -= dy; s->position.width += dx; break;
            case HandleType::BottomLeft:   s->position.x += dx; s->position.width -= dx; s->position.height += dy; break;
            default: break;
        }

        // 최소 크기 보장
        if (s->position.width < 30) s->position.width = 30;
        if (s->position.height < 20) s->position.height = 20;
    }

    Refresh();
}

void MainCanvas::OnLeftDClick(wxMouseEvent& evt) {
    wxPoint pos = evt.GetPosition();
    for (std::vector<Area*>::const_reverse_iterator it = areas.rbegin(); it != areas.rend(); ++it) {
        if((*it)->OpenProperty(pos)) return;
    }
}

void MainCanvas::ResizingShape(Shape* shape, HandleType& handleType){
    this->activeHandle = handleType;
    SelectShape(shape);
    resizing = true;
    // TODO
    // shapeTree->SetSelection(i);
    CaptureMouse();  // 마우스 캡처로 정확한 드래그 감지
    Refresh();
}

void MainCanvas::DraggingShape(Shape* shape, wxPoint& pt){
    dragStartMousePosition = pt;
    dragOffset = shape->GetPosition();
    SelectShape(shape);
    dragging = true;
    // TODO
    // shapeTree->SetSelection(i);
    showPreview = true;
    previewPos = SnapToGrid(dragOffset);
    CaptureMouse();
    Refresh();
}

void MainCanvas::AppendAreaToTree(wxTreeItemId parentId, Area* area) {
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

void MainCanvas::OnTreeSelectionChanged(wxTreeItemId itemId) {
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

void MainCanvas::OnTreeLeftDClick(wxTreeItemId itemId) {
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

void MainCanvas::OnFirstIdle(wxIdleEvent& evt) {
    wxSize size = GetClientSize();
    offset = wxPoint2DDouble(size.GetWidth() / 2, size.GetHeight() / 5);

    Unbind(wxEVT_IDLE, &MainCanvas::OnFirstIdle, this);  // 한 번만 실행되도록 해제
    Refresh();  // 화면 다시 그리기
}

void MainCanvas::UpdateAllShapesList() {
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

void MainCanvas::OnKeyDown(wxKeyEvent& evt) {
    if (evt.GetKeyCode() == WXK_SPACE) {
        spacePressed = true;
        evt.StopPropagation();
        SetCursor(wxCursor(wxCURSOR_HAND));
        return;
    }
    evt.Skip();
}

void MainCanvas::OnKeyUp(wxKeyEvent& evt) {
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

void MainCanvas::OnMiddleDown(wxMouseEvent& evt) {
    lastMouse = evt.GetPosition();
    middleMouseDown = true;
    StartPanning();
}

void MainCanvas::OnMiddleUp(wxMouseEvent& evt) {
    middleMouseDown = false;
    StopPanning();
}

void MainCanvas::StartPanning(){
    if(!panning){
        panning = true;
        SetCursor(wxCursor(wxCURSOR_HAND));
        CaptureMouse();
    }
}

void MainCanvas::StopPanning(){
    panning = false;
    SetCursor(wxCursor(*wxSTANDARD_CURSOR));
    if (HasCapture()) ReleaseMouse();
}

void MainCanvas::SelectShape(Shape* shape){
    UnSelectShape();
    selectedShape = shape;
    selectedShape->selected = true;
    for (const auto& [itemId, mappedShape] : shapeMap) {
        if (mappedShape == shape) {
            shapeTree->SelectItem(itemId);  // Tree에서 선택 처리
            break;
        }
    }
    Refresh();
}