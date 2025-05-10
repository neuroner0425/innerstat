#pragma once
#include <wx/wx.h>
#include <wx/listbox.h>
#include <vector>
#include "Shape.h"
#include "Area.h"
#include "Node.h"
#include "Connection.h"

class MyCanvas : public wxPanel {
public:
    /**
     * @brief MyCanvas 생성자 - 렌더링 및 이벤트 초기 설정
     */
    MyCanvas(wxWindow* parent, wxListBox* shapeList);

    /**
     * @brief MyCanvas 소멸자 - Shape 메모리 정리
     */
    ~MyCanvas();

    /**
     * @brief 새로운 Area 추가
     * @param areaType Area 종류 문자열
     */
    void AddNewArea(const std::string& areaType);

    /**
     * @brief 새로운 Node 추가
     * @param nodeType Node 종류 문자열
     */
    void AddNewNode(const std::string& nodeType);

    /**
     * @brief 특정 인덱스의 Shape 선택
     * @param index 선택할 Shape의 인덱스
     */
    void SelectShape(int index);

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

private:
    std::vector<Shape*> shapes;                // 도형 목록
    wxListBox* shapeList;                      // 리스트 UI 컨트롤

    size_t selectedIndex = -1;                 // 현재 선택된 도형 인덱스
    wxPoint lastMouse;                         // 마지막 마우스 위치
    double scale = 1.0;                        // 캔버스 확대/축소 배율
    wxPoint2DDouble offset = { 0, 0 };         // 화면 오프셋

    bool dragging = false;
    bool resizing = false;
    bool panning = false;
    HandleType activeHandle = HandleType::None;

    const Port* pendingPort = nullptr;         // 현재 연결 대기 중인 포트
    const Shape* pendingShape = nullptr;       // 연결 시작 도형
    std::vector<Connection> connections;       // 연결선 목록

    /**
     * @brief 리스트 UI를 도형 목록과 동기화
     */
    void RefreshList();

    /** @brief 도형 및 연결선 렌더링 */
    void OnPaint(wxPaintEvent& evt);

    /** @brief 마우스 휠 확대/축소 처리 */
    void OnMouseWheel(wxMouseEvent& evt);

    /** @brief 마우스 클릭(좌클릭) 처리 */
    void OnLeftDown(wxMouseEvent& evt);

    /** @brief 마우스 클릭 해제(좌클릭) 처리 */
    void OnLeftUp(wxMouseEvent& evt);

    /** @brief 마우스 드래그 처리 */
    void OnMotion(wxMouseEvent& evt);

    /** @brief 도형 더블클릭 시 속성창 열기 */
    void OnLeftDClick(wxMouseEvent& evt);

    /**
     * @brief 마우스 위치에서 포트 클릭 여부 확인
     * @param pos 마우스 위치
     * @param outShape 포트가 속한 도형 반환
     * @return 선택된 포트 포인터
     */
    const Port* HitTestPort(const wxPoint& pos, const Shape** outShape) const;

    wxDECLARE_EVENT_TABLE();
};