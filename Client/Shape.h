#pragma once
#include <wx/wx.h>
#include "Port.h"

class MainCanvas;
class Area;

/**
 * @brief 도형의 크기 조절을 위한 핸들 타입 정의
 */
enum class HandleType {
    None = -1,
    TopLeft, Top, TopRight,
    Left, Right,
    BottomLeft, Bottom, BottomRight
};

class Padding {
public:
    Padding(wxDouble size = 40){
        left = right = top = bottom = size / 2;
    };
    wxDouble left, right, top, bottom;
};

/**
 * @brief Area/Node의 공통 기반 클래스
 *        위치, 크기, 포트 정보와 도형 인터페이스를 정의
 */
class Shape {
public:
    wxPoint2DDouble pos;   // 도형의 좌상단 좌표
    double width;          // 도형 너비
    double height;         // 도형 높이
    MainCanvas* canvas = nullptr;
    Area* parent = nullptr;
    std::string label;
    
    bool selected = false;
    Padding padding;
    
    /**
     * @brief 생성자
     * @param x 도형 좌상단 X 좌표
     * @param y 도형 좌상단 Y 좌표
     * @param w 도형 너비
     * @param h 도형 높이
     */
    Shape(double x, double y, double w, double h, MainCanvas* canvas, Area* parent, const std::string& label);

    virtual ~Shape() = default;

    /**
     * @brief 도형을 화면에 그리는 순수 가상 함수
     * @param dc 그리기 대상 DC
     * @param scale 확대/축소 비율
     * @param offset 화면 오프셋
     * @param selected 선택 여부
     */
    virtual void Draw(wxDC& dc) const = 0;

    /**
     * @brief 속성 다이얼로그를 여는 가상 함수
     *        (Area/Node에서 오버라이드됨)
     */
    virtual void OpenPropertyDialog(MainCanvas* canvas) { }

    /**
     * @brief 주어진 화면 좌표가 도형 내부인지 검사
     * @param screenPt 마우스 위치 (스크린 좌표)
     * @param scale 현재 확대 배율
     * @param offset 캔버스 오프셋
     * @return true이면 도형 내부
     */
    virtual bool Contains(const wxPoint& screenPt) const;

    /**
     * @brief 도형을 직렬화하여 저장 가능한 문자열로 반환
     */
    virtual std::string Serialize() const = 0;

    /**
     * @brief 마우스 클릭 위치가 어떤 크기 조절 핸들에 속하는지 검사
     * @param screenPt 마우스 위치
     * @param scale 배율
     * @param offset 오프셋
     * @return 핸들 타입 (없으면 HandleType::None)
     */
    HandleType HitTestHandle(const wxPoint& screenPt) const;

    /**
     * @brief 도형이 보유한 포트 목록을 반환 (기본은 빈 목록)
     */
    virtual const std::vector<Port>& GetPorts() const {
        static std::vector<Port> empty;
        return empty;
    }

    virtual const Port* HitTestPort(const wxPoint& pos, const Shape** outShape) const;

    virtual bool HitTestShape(wxPoint& mouse);

    virtual bool OpenProperty(wxPoint& pos);
};
