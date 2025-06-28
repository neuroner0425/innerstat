#ifndef INNERSTAT_CLIENT_AREA_H
#define INNERSTAT_CLIENT_AREA_H

#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/base.h"
#endif

#include "innerstat/client/shape.h"
#include "innerstat/client/node.h"
#include "innerstat/client/port.h"
#include <string>
#include <vector>

INNERSTAT_BEGIN_NAMESPACE

class MainCanvas;
class Port;

class Area : public Shape {
public:
    AreaType type;
    std::vector<Port> ports;
    std::vector<Area*> childAreas;
    std::vector<Node*> childNodes;

    /** @brief 생성자
     * @param x X 좌표 @param y Y 좌표 @param w 너비 @param h 높이 @param type 시스템 타입
     */
    Area(int x, int y, int w, int h, 
        MainCanvas* canvas, Area* parent, const std::string& label, const AreaType& Type);

    /** @brief 소멸자 */
    ~Area();

    /** @brief 포트 수를 설정하고 재배치 */
    void SetPortCount(int count);

    /** @brief 포트 목록 반환 */
    const std::vector<Port>& GetPorts() const override { return ports; }

    /** @brief 도형을 그리는 함수 */
    void Draw(wxDC& dc) const override;

    /** @brief 직렬화된 문자열로 변환 */
    std::string Serialize() const;

    /** @brief 문자열로부터 Area 객체 생성 */
    static Area* Deserialize(const std::string& line, MainCanvas* canvas);

    /** @brief 자식 Area 추가 */
    void AddChildArea(Area* area);

    /** @brief 자식 Node 추가 */
    void AddChildNode(Node* node);

    /** @brief 자식 Area들 리스트 반환 */
    const std::vector<Area*>& GetSubAreas() const { return childAreas; };

    /** @brief 자식 Node들 리스트 반환 */
    const std::vector<Node*>& GetNodes() const { return childNodes; };

    /** @brief 주어진 좌표가 선택하고 있는 포트를 반환 */
    const Port* HitTestPort(const wxPoint& pos, const Shape** outShape) const override;

    /** @brief 주어진 좌표가 선택하고 있는 도형과 위치를 반환 */
    ShapeHandle HitTestShape(wxPoint& mouse) override;

    /** @brief Area의 종류를 std::string으로 반환*/
    inline const std::string getTypeStr() const{
        switch(type){
            case AreaType::OS : return "OS";
            case AreaType::VM : return "VM";
            case AreaType::Container : return "Container";
            case AreaType::Network : return "Network";
            default: return "Other";
        }
    }

    /** @brief 속성 편집 다이얼로그 열기 (이름, 포트 수 설정, 자식 추가) */
    void OpenPropertyDialog() override;

    /** @brief 자식 추가 다이얼로그 열기 */
    void OpenAddShapeDialog();
};
INNERSTAT_END_NAMESPACE

#endif