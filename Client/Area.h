#pragma once
#include "Shape.h"
#include "Node.h"
#include "Port.h"
#include <string>
#include <vector>

class MyCanvas;

/**
 * @brief Area 클래스는 시스템 단위 도형을 나타냅니다.
 *        타입(OS, VM 등)과 포트 정보를 포함하며, 시각화 및 직렬화 기능을 제공합니다.
 */
class Area : public Shape {
private:
    std::string type;               // 시스템 종류 (예: "OS", "VM", "HW")
    std::vector<Port> ports;       // 도형이 보유한 포트 목록
    std::vector<Area*> subAreas;
    std::vector<Node*> nodes;
public:
    inline std::string& getType(){
        return type;
    }

    /**
     * @brief 생성자
     * @param x X 좌표
     * @param y Y 좌표
     * @param w 너비
     * @param h 높이
     * @param type 시스템 타입
     */
    Area(double x, double y, double w, double h, const std::string& type, MyCanvas* parent);

    /** @brief 소멸자 */
    ~Area();

    /**
     * @brief 도형을 그리는 함수
     * @param dc DC 객체
     * @param scale 확대/축소 배율
     * @param offset 화면 오프셋
     * @param selected 선택 여부
     */
    void Draw(wxDC& dc) const override;

    /**
     * @brief 속성 편집 다이얼로그 열기 (이름, 포트 수 설정, 자식 추가)
     * @param parent 부모 윈도우
     */
    void OpenPropertyDialog(MyCanvas* parent) override;

    /**
     * @brief 자식 추가 다이얼로그 열기
     * @param parent 부모 윈도우
     */
    void OpenAddShapeDialog(MyCanvas* parent);

    /**
     * @brief 포트 수를 설정하고 재배치
     * @param count 포트 개수
     */
    void SetPortCount(int count);

    /**
     * @brief 포트 목록 반환
     */
    const std::vector<Port>& GetPorts() const override { return ports; }

    /**
     * @brief 직렬화된 문자열로 변환
     * @return 저장용 문자열 (예: Area OS 100 100 150 80 3)
     */
    std::string Serialize() const;

    /**
     * @brief 문자열로부터 Area 객체 생성
     * @param line 직렬화된 문자열
     * @return Area 객체 포인터
     */
    static Area* Deserialize(const std::string& line, MyCanvas* parent);

    void AddSubArea(Area* area);
    void AddNode(Node* node);

    const std::vector<Area*>& GetSubAreas() const;
    const std::vector<Node*>& GetNodes() const;


    const Port* HitTestPort(const wxPoint& pos, const Shape** outShape) const override;

    bool HitTestShape(wxPoint& mouse) override;

    bool OpenProperty(wxPoint& pos) override;
};
