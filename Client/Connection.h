#pragma once
#include "Port.h"
#include "Shape.h"

/**
 * @brief Connection 클래스는 두 포트 간의 연결선을 나타냅니다.
 *        연결 선은 꺾여 있는 (엘보 형태) 구조로, 포트와 도형에 대한 참조를 포함합니다.
 */
class Connection {
public:
    const Port* from;         // 시작 포트
    const Port* to;           // 도착 포트
    const Shape* fromShape;   // 시작 포트를 포함하는 도형
    const Shape* toShape;     // 도착 포트를 포함하는 도형

    /**
     * @brief 기본 생성자 (도형 정보 없음)
     */
    Connection(const Port* f, const Port* t)
        : from(f), to(t), fromShape(nullptr), toShape(nullptr) {}

    /**
     * @brief 도형 정보까지 포함하는 생성자
     */
    Connection(const Port* f, const Port* t, const Shape* fs, const Shape* ts)
        : from(f), to(t), fromShape(fs), toShape(ts) {}

    /**
     * @brief 연결선을 그리는 함수
     *        꺾인 선으로 시각화
     */
    void Draw(wxDC& dc, double scale, const wxPoint2DDouble& offset) const;
};
