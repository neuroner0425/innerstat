#ifndef INNERSTAT_CLIENT_AREA_H
#define INNERSTAT_CLIENT_AREA_H

#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/client.h"
#endif

#include "innerstat/client/port.h"
#include <string>
#include <vector>

#define REVERSE_FOR_AREA(it, container) \
    for (std::vector<innerstat::v1::Shape*>::const_reverse_iterator it = (container).rbegin(); it != (container).rend(); ++it)

INNERSTAT_BEGIN_NAMESPACE

class MainCanvas;
class Port;

class Shape {
public:
    wxRect rect;
    MainCanvas* canvas = nullptr;
    Shape* parent = nullptr;
    std::string label;
    
    bool isSelected = false;

    ShapeType type;
    std::vector<Port> ports;
    std::vector<Shape*> childAreas;

private:
    double* scale;
    wxPoint* offset;
    
public:
    /** @brief 생성자
     * @param x X 좌표 @param y Y 좌표 @param w 너비 @param h 높이 @param type 시스템 타입 */
    Shape(int x, int y, int w, int h, 
        MainCanvas* canvas, Shape* parent, const std::string& label, const ShapeType& Type, int portCount = 1);

    /** @brief 소멸자 */
    ~Shape();
    
    /** @brief 도형을 그리는 함수 */
    void Draw(wxDC& dc) const;
    
    
    /* ========== 자식과 관련된 함수들 ========== */
    
    /** @brief 자식 Shape 추가 */
    void AddChildArea(Shape* area);
    
    /** @brief 자식 Area들 리스트 반환 */
    inline const std::vector<Shape*>& GetChildAreas() const { return childAreas; };
    
    
    /* ========== 다이어로그 함수들 ========== */
    
    /** @brief 속성 편집 다이얼로그 열기 (이름, 포트 수 설정, 자식 추가) */
    void OpenPropertyDialog();
    
    /** @brief 자식 추가 다이얼로그 열기 */
    void OpenAddShapeDialog();
    
    
    /* ========== 도형 정보와 관련된 함수들 ========== */
    
    /** @brief 포트 수를 설정하고 재배치 */
    void SetPortCount(int count);
    
    /** @brief 포트 목록 반환 */
    const std::vector<Port>& GetPorts() const { return ports; }

    /** @brief 도형의 타입 반환 */
    inline ShapeType GetType() const { return type; }
    
    /** @brief Area의 종류를 std::string으로 반환*/
    inline const std::string GetTypeString() const{
        switch(GetType()){
            case ShapeType::PS : return "PS";
            case ShapeType::OS : return "OS";
            case ShapeType::VM : return "VM";
            case ShapeType::Container : return "Container";
            case ShapeType::Network : return "Network";
            default: return "Other";
        }
    }
    
    
    /* ========== 마우스 Hit과 관련된 함수들 ========== */
    
    /** @brief 주어진 좌표가 선택하고 있는 포트를 반환 */
    const Port* HitTestPort(const wxPoint& pos, const Shape** outShape) const;
    
    /** @brief 주어진 좌표가 선택하고 있는 도형과 위치를 반환 */
    ShapeHandle HitTestShape(wxPoint& mouse);
    
    /** @brief 주어진 화면 좌표가 도형 내부인지 검사 */
    inline bool Contains(const wxPoint& screenPt) const { return GetScreenRect().Contains(screenPt); }
    
    
    /* ========== 크기와 관련된 함수들 ========== */

    /** @brief 도형의 크기를 설정 */
    inline void SetSize(const wxSize& size) { rect.SetSize(size); }
    
    /** @brief 도형의 크기를 반환 */
    inline wxSize GetSize() const { return rect.GetSize(); }
    
    /** @brief 도형의 화면 크기를 반환 */
    inline wxSize GetScreenSize() const { return GetSize() * (*scale); }    
    
    
    /* ========== 위치와 관련된 함수들 ========== */
    
    /** @brief 도형의 위치를 설정 */
    inline void SetPosition(const wxPoint& pos) { rect.SetPosition(pos); }
    
    /** @brief 도형의 위치 변화를 설정 */
    inline void SetDeltaPosition(const wxPoint& pos) {
        for (auto area : GetChildAreas())
            area->SetDeltaPosition(pos);
        rect.SetPosition(pos + rect.GetPosition());
    }

    /** @brief 도형의 상대 위치를 반환 */
    inline wxPoint GetPosition() const { return rect.GetPosition(); }
    
    /** @brief 도형의 절대 위치를 반환 */
    inline wxPoint GetGridPosition() const { return rect.GetPosition() + (parent ? (parent->GetGridPosition() + wxPoint(GRID_SIZE/2, GRID_SIZE/2)) : wxPoint()); }

    /** @brief 도형의 화면 위치를 반환 */
    inline wxPoint GetScreenPosition() const {
        wxPoint pos = GetGridPosition();
        return wxPoint(pos.x * (*scale) + (*offset).x, pos.y * (*scale) + (*offset).y);
    }

    /** @brief 도형의 화면 사각형을 반환 */
    inline wxRect GetScreenRect() const { wxPoint screenPos = GetScreenPosition(); return wxRect(screenPos.x, screenPos.y, rect.width * (*scale), rect.height * (*scale)); }

    
    /* ========== 저장과 관련된 함수들 ========== */

    /** @brief 직렬화된 문자열로 변환 */
    std::string Serialize() const;

    /** @brief 문자열로부터 Shape 객체 생성 */
    static Shape* Deserialize(const std::string& line, MainCanvas* canvas);
};
INNERSTAT_END_NAMESPACE

#endif