#ifndef INNERSTAT_CLIENT_SHAPE_H
#define INNERSTAT_CLIENT_SHAPE_H

#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/base.h"
#endif

#include <wx/wx.h>

INNERSTAT_BEGIN_NAMESPACE

class MainCanvas;
class Area;
class Port;

class Shape {
public:
    wxRect rect;
    MainCanvas* canvas = nullptr;
    Area* parent = nullptr;
    std::string label;
    
    bool isSelected = false;
    
    /** @brief 생성자 */
    Shape(int x, int y, int w, int h, MainCanvas* canvas, Area* parent, const std::string& label);

    /** @brief 소멸자 */
    virtual ~Shape() = default;

    /** @brief 도형을 화면에 그리는 순수 가상 함수 */
    virtual void Draw(wxDC& dc) const = 0;

    /** @brief 속성 다이얼로그를 여는 가상 함수 */
    virtual void OpenPropertyDialog() { }

    /** @brief 주어진 화면 좌표가 도형 내부인지 검사 */
    virtual bool Contains(const wxPoint& screenPt) const;

    /** @brief 도형을 직렬화하여 저장 가능한 문자열로 반환 */
    virtual std::string Serialize() const = 0;

    /** @brief 도형이 보유한 포트 목록을 반환 */
    virtual const std::vector<Port>& GetPorts() const = 0;

    /** @brief 주어진 좌표가 선택하고 있는 포트를 반환 */
    virtual const Port* HitTestPort(const wxPoint& pos, const Shape** outShape) const;

    /** @brief 주어진 좌표가 선택하고 있는 도형과 위치를 반환 */
    virtual ShapeHandle HitTestShape(wxPoint& mouse);

    /** @brief 도형의 위치를 설정 */
    inline void SetPosition(const wxPoint& pos) { rect.SetPosition(pos); }

    /** @brief 도형의 위치를 반환 */
    inline wxPoint GetPosition() const { return rect.GetPosition(); }

    /** @brief 도형의 크기를 설정 */
    inline void SetSize(const wxSize& size) { rect.SetSize(size); }

    /** @brief 도형의 크기를 반환 */
    inline wxSize GetSize() const { return rect.GetSize(); }
};
INNERSTAT_END_NAMESPACE
#endif