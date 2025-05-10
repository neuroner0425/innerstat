#pragma once
#include "Shape.h"
#include "Port.h"
#include <vector>
#include <string>

/**
 * @brief Node 클래스는 말단 프로세스나 PID 단위를 시각화하는 도형입니다.
 *        상태(active/overloaded)와 포트, PID 식별자를 포함합니다.
 */
class Node : public Shape {
public:
    std::vector<Port> ports;       // 포트 목록
    std::string pidIdentifier;     // PID 또는 식별자
    bool active;                   // 활성 상태
    bool overloaded;               // 과부하 상태

    /**
     * @brief 생성자
     * @param x X 좌표
     * @param y Y 좌표
     * @param w 너비
     * @param h 높이
     * @param pid PID 문자열
     */
    Node(double x, double y, double w, double h, const std::string& pid);

    /**
     * @brief 도형 그리기
     * @param dc DC 객체
     * @param scale 확대/축소 배율
     * @param offset 오프셋
     * @param selected 선택 여부
     */
    void Draw(wxDC& dc, double scale, const wxPoint2DDouble& offset, bool selected) const override;

    /**
     * @brief 속성 설정 다이얼로그 열기
     * @param parent 부모 창
     */
    void OpenPropertyDialog(wxWindow* parent) override;

    /**
     * @brief 포트 개수 설정 및 재배치
     * @param count 포트 수
     */
    void SetPortCount(int count);

    /**
     * @brief 포트 목록 반환
     */
    const std::vector<Port>& GetPorts() const override { return ports; }

    /**
     * @brief Node 도형을 문자열로 직렬화
     */
    std::string Serialize() const;

    /**
     * @brief 문자열로부터 Node 객체 생성
     */
    static Node* Deserialize(const std::string& line);
};
