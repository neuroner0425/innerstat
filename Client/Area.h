#pragma once
#include "Shape.h"
#include "Node.h"
#include "Port.h"
#include <string>
#include <vector>

class MainCanvas;

enum class AreaType {
    None = 0,
    OS, VM, Container, Network
};

/**
 * @brief Area 클래스는 시스템 단위 도형을 나타냅니다.
 *        타입(OS, VM 등)과 포트 정보를 포함하며, 시각화 및 직렬화 기능을 제공합니다.
 */
class Area : public Shape {
public:
    AreaType type;               // 시스템 종류 (예: "OS", "VM", "HW")
    std::vector<Port> ports;       // 도형이 보유한 포트 목록
    std::vector<Area*> subAreas;
    std::vector<Node*> nodes;

    /**
     * @brief 생성자
     * @param x X 좌표
     * @param y Y 좌표
     * @param w 너비
     * @param h 높이
     * @param type 시스템 타입
     */
    Area(int x, int y, int w, int h, 
        MainCanvas* canvas, Area* parent, const std::string& label,
        const AreaType& Type);

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
     * @param canvas 부모 윈도우
     */
    void OpenPropertyDialog(MainCanvas* canvas) override;

    /**
     * @brief 자식 추가 다이얼로그 열기
     * @param canvas 부모 윈도우
     */
    void OpenAddShapeDialog(MainCanvas* canvas);

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
    static Area* Deserialize(const std::string& line, MainCanvas* canvas);

    void AddSubArea(Area* area);
    void AddNode(Node* node);

    const std::vector<Area*>& GetSubAreas() const;
    const std::vector<Node*>& GetNodes() const;


    const Port* HitTestPort(const wxPoint& pos, const Shape** outShape) const override;

    bool HitTestShape(wxPoint& mouse) override;

    bool OpenProperty(wxPoint& pos) override;
    
    inline const std::string getTypeStr() const{
        switch(type){
            case AreaType::OS : return "OS";
            case AreaType::VM : return "VM";
            case AreaType::Container : return "Container";
            case AreaType::Network : return "Network";
            default: return "other";
        }
    }

    inline int getTypeInt() const{
        switch(type){
            case AreaType::OS : return 1;
            case AreaType::VM : return 2;
            case AreaType::Container : return 3;
            case AreaType::Network : return 4;
            default: return 0;
        }
    }

    static inline AreaType getTypeByInt(int intT){
        switch(intT){
            case 1: return AreaType::OS;
            case 2: return AreaType::VM;
            case 3: return AreaType::Container;
            case 4: return AreaType::Network;
            default: return AreaType::None;
        }
    }
};
