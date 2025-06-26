#pragma once
#include "innerstat/client/shape.h"
#include "innerstat/client/port.h"
#include <vector>
#include <string>

class MainCanvas;
class Area;

class Node : public Shape {
public:
    std::vector<Port> ports;
    bool active;
    bool overloaded;

    /** @brief 생성자 */
    Node(int x, int y, int w, int h,
        MainCanvas* canvas, Area* parent, const std::string& label);

    /** @brief 도형 그리기 */
    void Draw(wxDC& dc) const override;

    /** @brief 속성 설정 다이얼로그 열기 */
    void OpenPropertyDialog() override;

    /** @brief 포트 개수 설정 및 재배치 */
    void SetPortCount(int count);

    /** @brief 포트 목록 반환 */
    const std::vector<Port>& GetPorts() const override { return ports; }

    /** @brief Node 도형을 문자열로 직렬화 */
    std::string Serialize() const;

    /** @brief 문자열로부터 Node 객체 생성 */
    static Node* Deserialize(const std::string& line, MainCanvas* canvas);
};
