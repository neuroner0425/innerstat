#ifndef INNERSTAT_CLIENT_PORT_H
#define INNERSTAT_CLIENT_PORT_H

#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/client.h"
#endif

#include <wx/wx.h>
#include <string>

INNERSTAT_BEGIN_NAMESPACE

class MainCanvas;

class Port {
private:
    MainCanvas* canvas = nullptr;
public:
    std::string id;
    wxPoint2DDouble relativePos;
    mutable wxPoint cachedScreenPos;

    /** @brief 생성자 */
    Port(MainCanvas* canvas, const std::string& id, const wxPoint2DDouble& relPos);

    /** @brief 포트의 화면 좌표 계산 및 캐시 */
    wxPoint GetScreenPosition(const wxRect& rect) const;

    /** @brief 포트를 화면에 원 형태로 그리기 */
    void Draw(wxGraphicsContext& gc, const wxRect& shapeRect) const;
};
INNERSTAT_END_NAMESPACE
#endif