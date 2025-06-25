#pragma once
#include "Port.h"
#include "Shape.h"

class Connection {
public:
    const Port* from;
    const Port* to;
    const Shape* fromShape;
    const Shape* toShape;

    /** @brief 기본 생성자 (도형 정보 없음) */
    Connection(const Port* f, const Port* t)
        : from(f), to(t), fromShape(nullptr), toShape(nullptr) {}

    /** @brief 도형 정보까지 포함하는 생성자 */
    Connection(const Port* f, const Port* t, const Shape* fs, const Shape* ts)
        : from(f), to(t), fromShape(fs), toShape(ts) {}

    /** @brief 연결선을 그리는 함수 */
    void Draw(wxDC& dc, double scale, const wxPoint2DDouble& offset) const;
};
