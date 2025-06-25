#pragma once
#include <wx/wx.h>
#include <wx/listbox.h>
#include <wx/treectrl.h>

#include <vector>
#include <map>

#include "Shape.h" // Shape 객체의 isSelected 변수를 접근하기 때문에 include

class Area;
enum class AreaType;
class Connection;
class MainFrame;

constexpr int GRID_SIZE = 40;

enum class UserAction{
    None = 0, connecting, dragging, resizing, panning
};

class MainCanvas : public wxPanel {
public:
    bool isdebug = true;

    double scale = 1.0;
    wxPoint offset = { 0, 0 };

private:
    MainFrame* frame;

    std::map<wxTreeItemId, Shape*> shapeMap;
    std::vector<Shape*> allShapes;

    std::vector<Connection> connections;
    std::vector<Area*> uppermostAreas;

    bool spacePressed = false;
    bool middleMouseDown = false;
    
    UserAction action = UserAction::None;
    HandleType activeHandle = HandleType::None;

    Shape* selectedShape = nullptr;
    
    wxPoint lastMouse;
    
    wxPoint actionOffset;
    wxPoint actionMousePos;

    wxRect previewRect;

    const Port* pendingPort = nullptr;
    const Shape* pendingShape = nullptr;

public:
    /** @brief MainCanvas 생성자 - 렌더링 및 이벤트 초기 설정 */
    MainCanvas(wxWindow* parent, MainFrame* frame);

    /** @brief MainCanvas 소멸자 - Shape 메모리 정리 */
    ~MainCanvas();

    /** @brief 최상단 Area 추가 */
    void AddNewArea(const std::string& label, const AreaType areaType);

    /** @brief 리스트 UI를 도형 목록과 동기화 */
    void RefreshTree();

    /** @brief 모든 Shape 및 Connection 정보를 파일로 저장 */
    void SaveToFile(const std::string& path);

    /** @brief 텍스트 파일로부터 도형 및 연결선 불러오기 */
    void LoadFromFile(const std::string& path);

    /** @brief */
    void UpdateAllShapesList();
    
    /** @brief */
    static wxPoint SnapToGrid(const wxPoint& pt);
    
private:
    /** @brief Shape의 포인터로 선택 */
    void SelectShape(Shape* shape);

    /** @brief 도형 선택 해제 */
    inline void UnSelectShape(){ if (selectedShape != nullptr) selectedShape->isSelected = false; selectedShape = nullptr; }

    /** @brief */
    void AppendAreaToTree(wxTreeItemId parentId, Area* area);

    /** @brief */
    void OnFirstIdle(wxIdleEvent& evt);

    /** @brief */
    void ResizingShape(Shape* shape, HandleType& handleType);

    /** @brief */
    void DraggingShape(Shape* shape, wxPoint& pt);

    /** @brief */
    void StartPanning();
    
    /** @brief */
    void StopPanning();
    
public:
    /** @brief */
    void OnTreeLeftDClick(wxTreeItemId itemId);

    /** @brief */
    void OnTreeSelectionChanged(wxTreeItemId itemId);

    /** @brief 키보드 눌림 처리 */
    void OnKeyDown(wxKeyEvent& evt);
    
    /** @brief 키보드 눌림 해제 처리 */
    void OnKeyUp(wxKeyEvent& evt) ;

private:
    /** @brief 도형 및 연결선 렌더링 */
    void OnPaint(wxPaintEvent& evt);

    /** @brief 마우스 휠 확대/축소 처리 */
    void OnMouseWheel(wxMouseEvent& evt);

    /** @brief 마우스 클릭(좌클릭) 처리 */
    void OnLeftDown(wxMouseEvent& evt);

    /** @brief 마우스 클릭 해제(좌클릭) 처리 */
    void OnLeftUp(wxMouseEvent& evt);

    /** @brief 마우스 클릭(휠클릭) 처리 */
    void OnMiddleDown(wxMouseEvent& evt);

    /** @brief 마우스 클릭 해제(휠클릭) 처리 */
    void OnMiddleUp(wxMouseEvent& evt);

    /** @brief 마우스 드래그 처리 */
    void OnMotion(wxMouseEvent& evt);

    /** @brief 도형 더블클릭 시 속성창 열기 */
    void OnLeftDClick(wxMouseEvent& evt);
};