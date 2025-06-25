#include <wx/dcbuffer.h>
#include <stdio.h>

#include <sstream>
#include <fstream>
#include <unordered_map>
#include <cmath>

#include "Canvas.h"
#include "Shape.h"
#include "Area.h"
#include "Connection.h"
#include "MainFrame.h"

MainCanvas::MainCanvas(wxWindow* parent, MainFrame* frame)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS), frame(frame) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &MainCanvas::OnPaint, this);
    Bind(wxEVT_MOUSEWHEEL, &MainCanvas::OnMouseWheel, this);
    Bind(wxEVT_LEFT_DOWN, &MainCanvas::OnLeftDown, this);
    Bind(wxEVT_LEFT_UP, &MainCanvas::OnLeftUp, this);
    Bind(wxEVT_MIDDLE_DOWN, &MainCanvas::OnMiddleDown, this);
    Bind(wxEVT_MIDDLE_UP, &MainCanvas::OnMiddleUp, this);
    Bind(wxEVT_KEY_UP, &MainCanvas::OnKeyUp, this);
    Bind(wxEVT_MOTION, &MainCanvas::OnMotion, this);
    Bind(wxEVT_LEFT_DCLICK, &MainCanvas::OnLeftDClick, this);
    Bind(wxEVT_IDLE, &MainCanvas::OnFirstIdle, this);
}

MainCanvas::~MainCanvas() {
    for (std::vector<Area*>::const_reverse_iterator it = uppermostAreas.rbegin(); it != uppermostAreas.rend(); ++it)
        delete (*it);
    uppermostAreas.clear();
}

void MainCanvas::AddNewArea(const std::string& label, const AreaType areaType) {
    uppermostAreas.push_back(new Area(120, 120 + uppermostAreas.size() * 40, 120, 120, this, nullptr, label, areaType));
    
    UpdateAllShapesList();
    RefreshTree();
    Refresh();
}

void MainCanvas::SaveToFile(const std::string& path) {
    // TODO
}

void MainCanvas::LoadFromFile(const std::string& path) {
    // TODO
}

void MainCanvas::RefreshTree() {
    frame->shapeTree->DeleteAllItems();
    shapeMap.clear();

    wxTreeItemId root = frame->shapeTree->AddRoot("Systems");

    for (size_t i = 0; i < uppermostAreas.size(); ++i) {
        if (Area* area = uppermostAreas[i]) {
            AppendAreaToTree(root, area);
        }
    }

    frame->shapeTree->ExpandAll();
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
    double mid_x = std::fmod(offset.x, GRID_SIZE * scale);
    double mid_y = std::fmod(offset.y, GRID_SIZE * scale);
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
    
    // 드래그 미리보기
    if (selectedShape && (action == UserAction::dragging)) {
        wxRect rect(previewRect);
        wxPoint screenPos(rect.x * scale + offset.x, rect.y * scale + offset.y);
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
    if (action == UserAction::connecting && pendingPort) {
        wxRect pendingShapePosition(pendingShape->position);
        wxPoint start = pendingPort->GetScreenPosition(
            pendingShapePosition.GetPosition(), pendingShapePosition.width, pendingShapePosition.height, scale, offset);
        wxPoint end = actionMousePos;
        dc.SetPen(wxPen(*wxBLACK, (int)(2 * scale), wxPENSTYLE_DOT));
        dc.DrawLine(start, end);
    }
    
    // 디버깅용
    if(isdebug){
        std::string actionStr("Unknown");
        switch (action) {
            case UserAction::None: actionStr =  "None"; break;
            case UserAction::connecting: actionStr =  "connecting"; break;
            case UserAction::dragging: actionStr =  "dragging"; break;
            case UserAction::resizing: actionStr =  "resizing"; break;
            case UserAction::panning: actionStr =  "panning"; break;
        }
        dc.SetBrush(wxBrush(*wxRED));
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawCircle(wxPoint((int)offset.x, (int)offset.y), (int)(scale * 8));
        dc.DrawText(wxString::Format("Scale: %.2f  ||  %s", scale, actionStr), wxPoint(0, 0));
        if(action != UserAction::None){
            dc.DrawText(wxString::Format("start_position(%d, %d)", actionMousePos.x, actionMousePos.y), wxPoint(0, 20));
            dc.DrawText(wxString::Format("current_position(%d, %d)", lastMouse.x, lastMouse.y), wxPoint(0, 40));
        }
    }
}


void MainCanvas::OnMouseWheel(wxMouseEvent& evt) {
    if(action != UserAction::None) return;
    // 휠 방향에 따라 배율 조정값 결정
    double factor = (evt.GetWheelRotation() > 0) ? 1.1 : 1.0 / 1.1;

    wxPoint mouse = evt.GetPosition();

    // 1. 확대/축소 전 마우스 위치의 "논리 좌표" 계산
    wxPoint2DDouble before((mouse.x - offset.x) / scale,
                           (mouse.y - offset.y) / scale);

    // 2. 배율 적용 (최소/최대 배율 제한 포함)
    scale *= factor;
    if (scale < 0.1) scale = 0.1;
    if (scale > 10.0) scale = 10.0;

    // 3. 확대/축소 후의 "논리 좌표" 계산
    wxPoint2DDouble after((mouse.x - offset.x) / scale,
                          (mouse.y - offset.y) / scale);

    // 4. 줌인/아웃 시 마우스 기준으로 캔버스 위치를 보정 (줌 중심 유지)
    offset.x += (after.m_x - before.m_x) * scale;
    offset.y += (after.m_y - before.m_y) * scale;

    // 5. 다시 그리기
    Refresh();
}

void MainCanvas::OnLeftDown(wxMouseEvent& evt) {
    wxPoint mouse = evt.GetPosition();
    actionMousePos = lastMouse = mouse;

    if (middleMouseDown) return;
    
    if (spacePressed) {
        StartPanning();
        return;
    }

    // 액션 초기화
    action = UserAction::None;

    // 1. 포트간 연결
    const Shape* clickedShape = nullptr;
    const Port* clickedPort = nullptr;

    for(std::vector<Area*>::const_reverse_iterator it = uppermostAreas.rbegin(); it != uppermostAreas.rend(); ++it)
        if((clickedPort = (*it)->HitTestPort(mouse, &clickedShape))) break;

    if (clickedPort) {
        pendingPort = clickedPort;
        pendingShape = clickedShape;
        action = UserAction::dragging;
        CaptureMouse();
        return;
    }

    // 2. 도형 선택 감지(도형 이동, 도형 크기 조절 작업)
    for(std::vector<Area*>::const_reverse_iterator it = uppermostAreas.rbegin(); it != uppermostAreas.rend(); ++it){
        ShapeHandle sh = (*it)->HitTestShape(mouse);
        if(sh.shape != nullptr && sh.handleType != HandleType::None){
            if (sh.handleType == HandleType::Body){
                DraggingShape(sh.shape, mouse);
            }
            else{
                if(selectedShape == sh.shape)
                    ResizingShape(sh.shape, sh.handleType);
                else
                    DraggingShape(sh.shape, mouse);
            }
            return;
        }
    }

    // 3. 아무 도형을 선택하지 않음
    UnSelectShape();
    frame->shapeTree->UnselectAll();

    Refresh();
}

void MainCanvas::OnLeftUp(wxMouseEvent& evt) {
    if (HasCapture()) ReleaseMouse();

    // 1. 포트간 연결
    if (action == UserAction::connecting) {
        wxPoint mouse = evt.GetPosition();
        const Shape* hoveredShape = nullptr;
        const Port* hoveredPort = nullptr;
        for(auto it = uppermostAreas.rbegin(); it != uppermostAreas.rend(); ++it)
            if((hoveredPort = (*it)->HitTestPort(mouse, &hoveredShape))) break;

        // 다른 포트 위에서 놓았을 때만 연결
        if (hoveredPort && hoveredPort != pendingPort) {
            connections.emplace_back(pendingPort, hoveredPort, pendingShape, hoveredShape);
        }
        action = UserAction::None;
        pendingPort = nullptr;
        pendingShape = nullptr;
        Refresh();
        return;
    }

    // 2. 패닝
    if (action == UserAction::panning) {
        action = UserAction::None;
        Refresh();
        return;
    }

    // 2. 도형 이동
    if (action == UserAction::dragging) {
        selectedShape->position = previewRect;
        action = UserAction::None;
        Refresh();
        return;
    }

    // 3. 도형 크기 조절
    if (action == UserAction::resizing) {
        action = UserAction::None;
        Refresh();
        return;
    }
}

void MainCanvas::OnMotion(wxMouseEvent& evt) {
    wxPoint diffPos = (evt.GetPosition() - actionMousePos);
    wxPoint delta = evt.GetPosition() - lastMouse;
    lastMouse = evt.GetPosition();

    // 1. 포트 간 연결
    if (action == UserAction::connecting) {
        actionMousePos = evt.GetPosition();
        Refresh(); // Paint에서 임시선 그림
        return;
    }

    // 2. 패닝     
    if ((spacePressed && evt.LeftIsDown() && evt.Dragging()) ||
        (middleMouseDown && evt.MiddleIsDown() && evt.Dragging())) {
        offset = diffPos + actionOffset;
        Refresh();
        return;
    }
    
    // 3. 도형 이동
    if (action == UserAction::dragging && selectedShape != nullptr) {
        diffPos.x /= scale;
        diffPos.y /= scale;
        diffPos += actionOffset;
        selectedShape->SetPosition(diffPos);
        previewRect.SetPosition(SnapToGrid(diffPos));
        Refresh();
        return;
    }

    // 4. 도형 크기 조절
    if (action == UserAction::resizing && selectedShape != nullptr) {
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
        Refresh();
        return;
    }
}

void MainCanvas::OnLeftDClick(wxMouseEvent& evt) {
    wxPoint pos = evt.GetPosition();
    for (std::vector<Area*>::const_reverse_iterator it = uppermostAreas.rbegin(); it != uppermostAreas.rend(); ++it) {
        ShapeHandle sh = (*it)->HitTestShape(pos);
        if(sh.shape != nullptr && sh.handleType != HandleType::None){
            sh.shape->OpenPropertyDialog();
            return;
        }
    }
}

void MainCanvas::ResizingShape(Shape* shape, HandleType& handleType){
    this->activeHandle = handleType;
    SelectShape(shape);
    action = UserAction::resizing;
    CaptureMouse();
    Refresh();
}

void MainCanvas::DraggingShape(Shape* shape, wxPoint& pt){
    actionMousePos = pt;
    actionOffset = shape->GetPosition();
    SelectShape(shape);
    previewRect = shape->position;
    action = UserAction::dragging;
    CaptureMouse();
    Refresh();
}

void MainCanvas::AppendAreaToTree(wxTreeItemId parentId, Area* area) {
    wxString label = wxString::Format("%s [%s]", area->label, area->getTypeStr());
    wxTreeItemId areaId = frame->shapeTree->AppendItem(parentId, label);
    shapeMap[areaId] = area;

    for (auto* sub : area->GetSubAreas()) {
        AppendAreaToTree(areaId, sub);
    }
    for (auto* node : area->GetNodes()) {
        wxString nlabel = wxString::Format("%s [PS]", node->label);
        wxTreeItemId nid = frame->shapeTree->AppendItem(areaId, nlabel);
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
                shape->OpenPropertyDialog();
                return;
            }
        }
    }
}

void MainCanvas::OnFirstIdle(wxIdleEvent& evt) {
    wxSize size = GetClientSize();
    offset = wxPoint(size.GetWidth() / 2, size.GetHeight() / 5);

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

    for (Area* a : uppermostAreas) {
        recurse(a);
    }
}

void MainCanvas::OnKeyDown(wxKeyEvent& evt) {
    if (evt.GetKeyCode() == WXK_SPACE) {
        SetFocus();
        if(!spacePressed){
            printf("패닝 시작!");
        }
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
    actionMousePos = lastMouse = evt.GetPosition();
    middleMouseDown = true;
    StartPanning();
}

void MainCanvas::OnMiddleUp(wxMouseEvent& evt) {
    middleMouseDown = false;
    StopPanning();
    Refresh();
}

void MainCanvas::StartPanning(){
    if(action != UserAction::panning){
        action = UserAction::panning;
        actionOffset = offset;
        SetCursor(wxCursor(wxCURSOR_HAND));
        CaptureMouse();
    }
}

void MainCanvas::StopPanning(){
    action = UserAction::None;
    if(!spacePressed) SetCursor(wxCursor(*wxSTANDARD_CURSOR));
    if (HasCapture()) ReleaseMouse();
}

void MainCanvas::SelectShape(Shape* shape){
    UnSelectShape();
    selectedShape = shape;
    selectedShape->isSelected = true;
    for (const auto& [itemId, mappedShape] : shapeMap) {
        if (mappedShape == shape) {
            frame->shapeTree->SelectItem(itemId);  // Tree에서 선택 처리
            break;
        }
    }
    Refresh();
}