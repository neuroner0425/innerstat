#ifndef INNERSTAT_CLIENT_CONNECTION_H
#define INNERSTAT_CLIENT_CONNECTION_H

#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/client.h"
#endif
#include "innerstat/client/port.h"
#include "innerstat/client/shape.h"

INNERSTAT_BEGIN_NAMESPACE

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

INNERSTAT_END_NAMESPACE
#endif