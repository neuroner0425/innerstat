#pragma once
#include <wx/wx.h>
#include <wx/listbox.h>
#include <wx/treectrl.h>

#include <vector>
#include <map>

#include "Shape.h"
#include "Area.h"
#include "Node.h"
#include "Connection.h"

constexpr int GRID_SIZE = 40;

class MainCanvas : public wxPanel {
public:
    bool isdebug = true;
    Shape* selectedShape = nullptr;                 // 현재 선택된 도형 인덱스
    wxPoint lastMouse;                         // 마지막 마우스 위치
    double scale = 1.0;                        // 캔버스 확대/축소 배율
    wxPoint2DDouble offset = { 0, 0 };         // 화면 오프셋
    
    /**
     * @brief MainCanvas 생성자 - 렌더링 및 이벤트 초기 설정
     */
    MainCanvas(wxWindow* parent, wxTreeCtrl* shapeTree);

    /**
     * @brief MainCanvas 소멸자 - Shape 메모리 정리
     */
    ~MainCanvas();

    /**
     * @brief 새로운 Area 추가
     * @param areaType Area 종류 문자열
     */
    void AddNewArea(const std::string& label, const AreaType areaType);

    /**
     * @brief Shape의 포인터로 선택
     * @param shape 선택할 Shape의 포인터
     */
    void SelectShape(Shape* shape);
    
    inline void UnSelectShape(){
        if (selectedShape != nullptr) selectedShape->selected = false;
        selectedShape = nullptr;
    }

    /**
     * @brief 리스트 UI를 도형 목록과 동기화
     */
    void RefreshTree();

    /**
     * @brief 모든 Shape 및 Connection 정보를 파일로 저장
     * @param path 저장할 파일 경로
     */
    void SaveToFile(const std::string& path);

    /**
     * @brief 텍스트 파일로부터 도형 및 연결선 불러오기
     * @param path 불러올 파일 경로
     */
    void LoadFromFile(const std::string& path);

    /**
     * 
     */
    void ResizingShape(Shape* shape, HandleType& handleType);

    /**
     * 
     */
    void DraggingShape(Shape* shape, wxPoint& pt);

    /**
     * 
     */
    void OnTreeSelectionChanged(wxTreeItemId itemId);  // 선택 연동

    
    /**
     * 
     */
    void OnTreeLeftDClick(wxTreeItemId itemId);  // 선택 연동

    void UpdateAllShapesList();
    
    static wxPoint SnapToGrid(const wxPoint& pt);

private:
    std::vector<Area*> areas;                // 도형 목록
    wxTreeCtrl* shapeTree;                      // 리스트 UI 컨트롤
    std::map<wxTreeItemId, Shape*> shapeMap;

    std::vector<Shape*> allShapes;

    bool spacePressed = false;    // 스페이스바 상태
    bool middleMouseDown = false; // 휠클릭 상태

    bool isDrawingConnection = false;
    wxPoint pendingMousePos; // 마우스 현재 위치
    
    bool dragging = false;
    bool resizing = false;
    bool panning = false;
    HandleType activeHandle = HandleType::None;
    
    wxPoint dragOffset;        // 마우스-도형 좌상단 거리
    wxPoint dragStartMousePosition;

    // 미리보기 상태
    bool showPreview = false;
    wxPoint previewPos;        // 미리보기 위치(스냅 적용된)

    const Port* pendingPort = nullptr;         // 현재 연결 대기 중인 포트
    const Shape* pendingShape = nullptr;       // 연결 시작 도형
    std::vector<Connection> connections;       // 연결선 목록

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

    /** @brief 키보드 눌림 처리 */
    void OnKeyDown(wxKeyEvent& evt);
    
    /** @brief 키보드 눌림 해제 처리 */
    void OnKeyUp(wxKeyEvent& evt) ;

    /** @brief 마우스 드래그 처리 */
    void OnMotion(wxMouseEvent& evt);

    /** @brief 도형 더블클릭 시 속성창 열기 */
    void OnLeftDClick(wxMouseEvent& evt);

    void AppendAreaToTree(wxTreeItemId parentId, Area* area);

    void OnFirstIdle(wxIdleEvent& evt);

    void StartPanning();
    
    void StopPanning();
};